#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>


/* Loader for GM 19.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "loader.h"
#include "modelstore.h"
#include "configuration.h"
#include "tokenizer.h"
#include "reader.h"

#define CONFIG_EXTENSION ".config"

static FILE *_loaderFile = NULL;

typedef GObject* (*CreateObject)(const char *name);
typedef int (*ParameterReader)(GObject *object, const char* property, char *value, MeaUnit unit);

typedef struct StructureT {
    const char* name;
    size_t name_size;    
    CreateObject create;
    ParameterReader read_param;
} Structure;


static int _loaderSaveSolid(Solid *solid);
static int _loaderSaveMesh(Mesh *mesh);

static CommonErrorHandler loader_error_handler = NULL;


void initLoader(CommonErrorHandler error_handler)
{        
    loader_error_handler = error_handler;
    initReader(error_handler);
}

void loaderPrintError(int err, const char* text, ...)
{
    char textBuffer[256];
    textBuffer[0] = 0;
    
    va_list param;
    va_start(param, text);    
    vsnprintf(textBuffer, sizeof(textBuffer), text, param);    
    va_end(param);    
    
    if (loader_error_handler)
    {
        loader_error_handler(err,textBuffer);
    }
    else
    {
        fputs(textBuffer, stderr);
    }    
}


int loaderLoadModel(Model **model, const char *fileName)
{
    LOG("Reading model from \"%s\".\n",fileName);

    _loaderFile = fopen(fileName,"r");
    
    if (NULL != _loaderFile)
    {
        *model = NULL;
        resetTokenizer(_loaderFile);

        ReaderScanner scanner;
        memset(&scanner,0,sizeof(scanner));
        
        scanner.scanner = tokenizerToken;
        scanner.push_back = tokenizerPushBack;
        
        for (const GObject *object = readerCreate(&scanner); 
             !feof(_loaderFile) && (NULL != object); 
             object = readerCreate(&scanner))
        {    
#ifdef _DEBUG_LOADER            
               char s1[GM_VERTEX_BUFFER_SIZE];
               LOG("Read: \"%s\"\n",vertexPath(object,s1,sizeof(s1)));
#endif               
               if (OBJ_MODEL == object->type)
               {
                   if (*model)
                   {
                       loaderPrintError(0,"Too many models in file!");
                   }
                   
                   *model = (Model*)object;                   
               }
        }
        
        fclose(_loaderFile);
        _loaderFile = NULL;
        resetTokenizer(_loaderFile);  
                
        return 0;
    }
    
    return -1;
}


int loaderSaveModel(Model *model, const char *fileName)
{
    if (!isObject((GObject*)model))
    {
        loaderPrintError(errno,"No model. Nothing to save!\n");
        return -1;
    }
    
    _loaderFile = fopen(fileName,"w");
    
    if (!commenIsTime(&model->created))
    {
        commenNow(&model->created);
    }

    commenNow(&model->last_modified);

    const char *objname = objectName(OBJ_MODEL);
    
    if ((NULL != _loaderFile) && objname)
    {
        char timeCreated[100];
        char timeModified[100];
        
        strftime(timeCreated, sizeof(timeCreated), GM_TIME_FORMAT,&model->created);
        strftime(timeModified, sizeof(timeModified), GM_TIME_FORMAT,&model->last_modified);

        fprintf(_loaderFile,"# File created by %s (V:%s) of Georgi Bootbau\n",GM_APPLICATION,GM_VERSION);
        fprintf(_loaderFile,"%s(\"%s\"):\nowner(\"%s\");\nat(%g,%g,%g);created(\"%s\");last_modified(\"%s\")\n",
                objname,
                model->name,model->owner,
                model->p[0],model->p[1],model->p[2],timeCreated,timeModified);

        for (CObject *object = model->first; NULL != object; object = isCObject(ANY_COBJECT,object->next))
        {     
            Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
            if (solid)
            {
                _loaderSaveSolid(solid);
            }
            else
            {
                Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
                if (mesh)
                {
                    _loaderSaveMesh(mesh);
                }                    
            }
            
        }
        
        fclose(_loaderFile);
    }
    else
    {
        loaderPrintError(errno,"Failed to open \"%s\"\n",fileName);
    }
    
    _loaderFile = NULL;
    
    return 0;
}

static int _loaderSavePolygon(Polygon *poly)
{
    if (isObject((GObject*)poly))
    {
        const char *objname = objectName(OBJ_POLYGON);
        const char *vobjname = objectName(OBJ_VERTEX);
        
        if (objname && vobjname)
        {
            // No need to persist these flags
            flag_type flags = poly->flags & ~(GM_FLAG_MASK_NOT_PERSISTENT);
            
            fprintf(_loaderFile,"%s%i:at(%g, %g, %g);0x%LX;",objname,objectGetIndex((GObject*)poly),poly->p[0],poly->p[1],poly->p[2],flags);
            
            for (Vertex *vertex = poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
            {           
                IndexType index = objectGetIndex((GObject*)vertex);
                char stringBuffer[256];
                vertexToString(vertex, stringBuffer, sizeof(stringBuffer),GM_V2S_FLAGS_FILE);
                fprintf(_loaderFile,"%s%i%s",vobjname,index,stringBuffer);

                if (NULL != vertex->next)
                {            
                    if (0 == (index % 3))
                    {
                        fprintf(_loaderFile,";\n");
                    }
                    else
                    {
                        fprintf(_loaderFile,";");
                    }
                }
                else
                {
                    fprintf(_loaderFile,"\n");
                }            
            }
            return 0;
        }
    }
    
    return -1;
}

static int _loaderSaveMaterial(Material *material)
{
   const char *objname = objectName(OBJ_MATERIAL);
    
   fprintf(_loaderFile,"%s(\"%s\");%g;0x%lX;%g\n",objname,material->name,material->min_density,material->color,material->min_thick);
   
   return 0;
}

static int _loaderSaveComments(const char* comments)
{
   fprintf(_loaderFile,"comment;\"%s\";comment_end;\n",comments);
     
   return 0;
}


static int _loaderSaveSolid(Solid *solid)
{
    if (isObject((GObject*)solid))
    {
        const char *objname = objectName(OBJ_SOLID);
        
        // No need to persist these flags
        flag_type flags = solid->flags & ~(GM_FLAG_MASK_NOT_PERSISTENT);

        fprintf(_loaderFile,"%s%i(\"%s\"):at(%g,%g,%g);0x%llX\n",objname,objectGetIndex((GObject*)solid),
                solid->name,
                solid->p[0],solid->p[1],solid->p[2],
                flags);
        
        if (solid->comments)
        {
            _loaderSaveComments(solid->comments);
        }
        
        if (solid->material)
        {
            _loaderSaveMaterial(solid->material);
        }
        
        for (Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
        {    
            _loaderSavePolygon(poly);
        }

        objname = objectName(OBJ_MEASUREMENT);
        
        for (Mea *mea = (Mea*)isCObject(OBJ_MEASUREMENT,solid->first_mea); NULL != mea; mea = (Mea*)mea->next)
        {            
            for (int i = 0; i < mea->number_of_values; i++)
            {
                if (ValueType_Connection == mea->values[i].vtype)
                {
                    Value *value = &mea->values[i];


                    Polygon *poly1 = (Polygon *)isGObject(OBJ_POLYGON,value->data[value->value_index].connection.x1->parent);
                    Polygon *poly2 = (Polygon *)isGObject(OBJ_POLYGON,value->data[value->value_index].connection.x2->parent);
                    
                    const IndexType p1 = poly1->index;
                    const IndexType p2 = poly2->index;
                    const IndexType x1 = value->data[value->value_index].connection.x1->index;
                    const IndexType x2 = value->data[value->value_index].connection.x2->index;
                    
                    fprintf(_loaderFile,"%s%i(%s):connect(Poly%i,V%i,Poly%i,V%i);unit(%s)\n",
                            objname,
                            objectGetIndex((GObject*)mea),
                            value->name,
                            p1,x1,
                            p2,x2,
                            meaUnitToString(value->unit));
                }
            }
        }
        
        return 0;
    }
    
    return -1;
}    

static int _loaderSaveMesh(Mesh *mesh)
{
    (void)mesh;
    return -1;
}

const char* loaderGetConfigNameFromModel(const char* fileName, char* buffer, int bufferSize)
{
    strncpy(buffer,fileName,bufferSize);
    
    char *configFileName = buffer;
    char *postFix = NULL;
    int restBufferSize = bufferSize;
    
    for (char *i = buffer;*i;i++)
    {
        if ('.' == *i)
        {
            *i = 0;
            postFix = i;
            break;
        }
        
        restBufferSize--;
    }
    
    if (postFix && (restBufferSize > (int)sizeof(CONFIG_EXTENSION)))
    {
        strcpy(postFix,CONFIG_EXTENSION);
    }
    else
    {
        strcpy(buffer,fileName);
        commenStringCat(buffer,CONFIG_EXTENSION,bufferSize);
    }
    
    return configFileName;
}

int loaderReadFrameConfig(GObject *object, const char* property, char *value, MeaUnit unit)
{
    (void)object;
    (void)property;
    (void)value;
    (void)unit;
    
    return 0;
}

int loaderReadEquipmentConfig(GObject *object, const char* property, char *value, MeaUnit unit)
{
    (void)object;
    (void)property;
    (void)value;
    (void)unit;
    
    return 0;
}

int loaderReadMaterialConfig(GObject *object, const char* property, char *value, MeaUnit unit)
{
    Material *material = (Material*)isGObject(OBJ_MATERIAL,object);
    if (material)
    {
        if (strncmp(property,"density",sizeof("density")) == 0)
        {
            int array_length = 0;
            double *density = configStringToDoubleArray(commenStripString(value), &array_length);

            if (density)
            {
                material->densityUnit = unit;
                material->min_density = density[0];
                if (array_length > 1)
                {
                    material->max_density = density[1];
                }
                else
                {
                    material->max_density = material->min_density;
                }
                
                memory_free(density);
            }
        }
        else
        if (strncmp(property,"thick",sizeof("thick")) == 0)
        {
            int array_length = 0;
            double *thick = configStringToDoubleArray(commenStripString(value), &array_length);

            if (thick)
            {
                material->thickUnit = unit;
                material->min_thick = thick[0];
                if (array_length > 1)
                {
                    material->max_thick = thick[1];
                }
                else
                {
                    material->max_thick = material->min_thick;
                }
                
                memory_free(thick);
            }
        }
        else
        if (strncmp(property,"color",sizeof("color")) == 0)
        {
            material->color = commenStringToULong(commenStripString(value));
        }
        else
        if (strncmp(property,"spec",sizeof("spec")) == 0)
        {
            strncpy(material->spec,value,sizeof(material->spec));
        }
        else
        {
            return -1;
        }

        return 0;
    }
    
    return -1;
}


static GObject *createMaterial(const char *name)
{    
    return (GObject*) modelstoreCreateMaterial(NULL, name);                    
}

static GObject *createFrame(const char *name)
{
    (void)name;
    LOG("Try to create frma %s. Not yet implemented!\n",name);
    return (GObject*) NULL;                    
}

GObject *createEquipment(const char *name)
{
    (void)name;
    LOG("Try to create equipment %s. Not yet implemented!\n",name);
    return (GObject*) NULL;                    
}


int loaderLoadConfig(const char *fileName)
{
    const Structure objects[] = {
        { "material",  sizeof("material")-1,createMaterial,  loaderReadMaterialConfig },
        { "frame",  sizeof("frame")-1,createFrame,  loaderReadFrameConfig },
        { "equipment",  sizeof("equipment")-1,createEquipment,  loaderReadEquipmentConfig },
    };
        
    _loaderFile = fopen(fileName,"r");
    
    if (NULL != _loaderFile)
    {
        resetTokenizer(_loaderFile);
        
        char nameBuffer[128];
        char valueBuffer[512];
        
        LOG("Reading config from \"%s\"\n",fileName);

        GObject *structure = NULL;
        ParameterReader read_param = NULL;
        
        char structureName[MODEL_NAME_SIZE];
        structureName[0] = 0;
        
        for (char* tok = tokenizerToken(); NULL != tok; tok = tokenizerToken())
        {    
            if (!commenIsLabel(tok))
            {
                loaderPrintError(errno,"Invalid config variable \"%s\".\n",tok);
                continue;
            }
            
            commenCopyString(nameBuffer,tok,sizeof(nameBuffer));

            const char* structure_prop = NULL;

            for (unsigned  i = 0; i < sizeof(objects)/sizeof(objects[0]);i++)
            {
                if (0 == strncasecmp(nameBuffer,objects[i].name,objects[i].name_size))
                {
                    char *name = NULL;
                    char *sep = strchr(nameBuffer,'.');
                    if (sep)
                    {
                        name = sep+1;
                        
                        sep = strchr(name,'.');
                        if (sep)
                        {
                            structure_prop = sep+1;
                            *sep = 0;
                        }
                    }
                    
                    if (name)
                    {
                        if ((NULL == structure) || (structure && (0 != strncmp(structureName,name,sizeof(structureName)))))
                        {
                            strncpy(structureName,name,sizeof(structureName));
                            structure = objects[i].create(name);
                            read_param = objects[i].read_param;
                            // material = modelstoreCreateMaterial(NULL, name);                    
                        }
                    }
                    
                    break;
                }
            }
            
            char *value = tokenizerToken();
            strcpy(valueBuffer,value);
            MeaUnit unit = MeaUnit_None;
            tok = tokenizerToken();
            if (tok)
            {
                unit = meaFindUnit(commenStripString(tok));
                
                if (!commenIsLabel(tok))
                {
                    if (MeaUnit_None == unit)
                    {
                        loaderPrintError(errno,"Invalid config variable \"%s\".\n",tok);
                    }
                }
                else
                {
                    if (MeaUnit_None == unit)
                    {                        
                        tokenizerPushBack(tok);
                    }
                }
            }

            if (value)
            {
                if (structure_prop && structure)
                {
#ifdef _DEBUG_CONFIG
                     LOG("Adding \"%s\" to \"%s\"\n",structure_prop,structureName);
#endif                     
                     
                     if (read_param)
                     {
                        if (read_param(structure, structure_prop, valueBuffer,unit))
                        {
                            LOG("Warning: Failed to add \"%s\" to \"%s\"\n",structure_prop,structureName);
                        }
                     }
                }
                else
                {
                    Config *config = configCreate(nameBuffer,unit);
                    if (config)
                    {
                        config->string = memory_strdup(commenStripString(valueBuffer));
                        // fprintf(stderr,"read: %s = \"%s\"\n",nameBuffer,config->string);                    
                        configStoreConfig(config);
                    }
                    else
                    {
                        loaderPrintError(errno,"Error: Could not read complete configuration\n");
                        break;
                    }
                }                
            }
            else
            {
                loaderPrintError(errno,"Value missing for \"%s\"\n",nameBuffer);
                break;
            }
        }
        
        fclose(_loaderFile);
        _loaderFile = NULL;        
        resetTokenizer(_loaderFile);
    }
    else
    {
        loaderPrintError(errno,"No config file \"%s\" found.\n",fileName);
        perror("Unable to open file.\n");
    }
    
    return 0;
}
