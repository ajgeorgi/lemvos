#include <string.h>
#include <stdio.h>

/* Reader for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "reader.h"
#include "modelstore.h"
#include "loader.h"
#include "mea.h"
#include "commen.h"

#define KWORD_NAME 1
#define KWORD_OWNER 2
#define KWORD_CREATED 3
#define KWORD_LAST_MODIFIED 4
#define KWORD_AT 5
#define KWORD_UNIT 6
#define KWORD_CONNECT 7
#define KWORD_COMMENT_END 8
#define KWORD_COMMENT 9

typedef struct ObjectTableT {
   ObjectType type;
   ReadObject reader;
} ObjectTable;

static ObjectTable _objectTable[10];
static unsigned int _object_table_index = 0;
static CommonErrorHandler reader_error_handler = NULL;
static Model *_reader_model = NULL;
static Solid *_reader_solid = NULL;
static Polygon *_reader_polygon = NULL;

const GObject *readerReadModel(IndexType index, ReaderScanner* scanner);
const GObject *readerReadSolid(IndexType index, ReaderScanner* scanner);
const GObject *readerReadMaterial(IndexType index, ReaderScanner* scanner);
const GObject *readerReadPolygon(IndexType index, ReaderScanner* scanner);
const GObject *readerReadVertex(IndexType index, ReaderScanner* scanner);
const GObject *readerReadMeasurement(IndexType index, ReaderScanner* scanner);

char *readFromScanner(ReaderScanner *scanner)
{
    if (scanner)
    {
        if (scanner->scanner)
        {
            return scanner->scanner();
        }
    }
    
    return NULL;
}

int pushBackToScanner(ReaderScanner *scanner, const char* token)
{
    if (scanner)
    {
        if (scanner->push_back)
        {
            return scanner->push_back(token);
        }
    }
    
    return -1;
}

void initReader(CommonErrorHandler error_handler)
{
    reader_error_handler = error_handler;
    
    readerRegisterObject(OBJ_MODEL, readerReadModel);
    readerRegisterObject(OBJ_SOLID, readerReadSolid);
    readerRegisterObject(OBJ_MATERIAL, readerReadMaterial);
    readerRegisterObject(OBJ_POLYGON, readerReadPolygon);
    readerRegisterObject(OBJ_VERTEX, readerReadVertex);
    readerRegisterObject(OBJ_MEASUREMENT, readerReadMeasurement);
}

int readerRegisterObject(ObjectType type, ReadObject reader)
{
    if (_object_table_index < sizeof(_objectTable)/sizeof(_objectTable[0]))
    {
        _objectTable[_object_table_index].type = type;
        _objectTable[_object_table_index].reader = reader;
        
        _object_table_index++;
        
        return 0;
    }
    
    return -1;
}

const GObject *readerCreate(ReaderScanner *scanner)
{
    const GObject *object = NULL;

    if (scanner)
    {
        IndexType index = 0;
        char *tok = readFromScanner(scanner);
        ObjectType type = objectType(tok,&index);
        if (0 == type)
        {
            ERROR("Unknown object \"%s\" in file!!\n",tok);
        }
        else
        {
            char oName[50];
            strncpy(oName,tok,sizeof(oName));
            
            ReadObject reader = NULL;
            
            for (unsigned int i = 0; i < sizeof(_objectTable)/sizeof(_objectTable[0]); i++)
            {
                if (type == _objectTable[i].type)
                {
                    reader = _objectTable[i].reader;
                    break;
                }
            }
            
            if (reader)
            {
                object = reader(index, scanner);
                
                if (NULL == object)
                {
                    LOG1("No more objects found!\n");
                }
            }
            else
            {
                const char *oname = objectName(type);
                ERROR("Could not find a reader for type %i (\"%s\"/\"%s\")\n",type,oname?oname:UNKNOWN_SYMBOL,oName);
            }
            
            if (object)
            {
                if (0 == object->index)
                {
                    LOG("Missing index for \"%s\"!\n",oName);
                }  
                if (NULL == object->parent)
                {
                    LOG("Parent missing for \"%s\"!\n",oName);
                }
                if (0 == object->type)
                {
                    LOG("Type missing for \"%s\"!\n",oName);
                }
            }
        }
    }
    
    return object;
}

static int kwordType(char *keyw)
{
    if (keyw)
    {
        if (0 == strncmp(keyw,"name",sizeof("name")-1))
        {
            return KWORD_NAME;
        }
        else
        if (0 == strncmp(keyw,"owner",sizeof("owner")-1))
        {
            return KWORD_OWNER;
        }
        else
        if (0 == strncmp(keyw,"created",sizeof("created")-1))
        {
            return KWORD_CREATED;
        }
        else
        if (0 == strncmp(keyw,"last_modified",sizeof("last_modified")-1))
        {
            return KWORD_LAST_MODIFIED;
        }
        else
        if (0 == strncmp(keyw,"at",sizeof("at")-1))
        {
            return KWORD_AT;
        }
        else
        if (0 == strncmp(keyw,"connect",sizeof("connect")-1))
        {
            return KWORD_CONNECT;
        }
        else
        if (0 == strncmp(keyw,"unit",sizeof("unit")-1))
        {
            return KWORD_UNIT;
        }  
        else
        if (0 == strncmp(keyw,"comment_end",sizeof("comment_end")-1))
        {
            return KWORD_COMMENT_END;
        }          
        else
        if (0 == strncmp(keyw,"comment",sizeof("comment")-1))
        {
            return KWORD_COMMENT;
        }          
    }
    
    return 0;
}


const GObject *readerReadModel(IndexType index, ReaderScanner* scanner)
{
    _reader_model = NULL;
    
    if (scanner)
    {
        char namebuffer[sizeof(_reader_model->name)];
        char ownerbuffer[sizeof(_reader_model->owner)];
        namebuffer[0] = 0;
        ownerbuffer[0] = 0;

        char *name = commenStripString(readFromScanner(scanner));
        commenCopyString(namebuffer,name,sizeof(namebuffer));
        char *owner = NULL;
        char *keyword = readFromScanner(scanner);
        if (KWORD_OWNER == kwordType(keyword))
        {
            owner = commenStripString(readFromScanner(scanner));
            commenCopyString(ownerbuffer,owner,sizeof(ownerbuffer));
            
            _reader_model = modelstoreCreate(namebuffer, ownerbuffer);
        }
        
        if (_reader_model)
        {
            _reader_model->index = index;
            keyword = readFromScanner(scanner);
            if (KWORD_AT == kwordType(keyword))
            {
                char *position = readFromScanner(scanner);
                
                commenVertexFromString(position,_reader_model->p,NULL);
            }

            keyword = readFromScanner(scanner);
            if (KWORD_CREATED == kwordType(keyword))
            {
                // Get the creation date from file
                char *createTime = readFromScanner(scanner);
               if (0 > commenTimeFromString(commenStripString(createTime),& _reader_model->created))
                {   
                    loaderPrintError(0,"Time conversion trouble!\n");
                }                           
            }

            keyword = readFromScanner(scanner);
            if (KWORD_LAST_MODIFIED == kwordType(keyword))
            {
                // Just swallow a symbol
                readFromScanner(scanner);
                // Just skip the token and get the real modification time of the OS                                
                if (0 > commenGetFileModTime(FILEMODTIME_CURRENT_FILENAME, &_reader_model->last_modified))
                {   
                    loaderPrintError(0,"Time conversion trouble!\n");
                }            
            }
            
            LOG("Created: %s, Modified: %s\n",asctime(& _reader_model->created),asctime(&_reader_model->last_modified));
        }
        else
        {
            loaderPrintError(0,"Could not find a model\n");
        }
    }
    if (_reader_model)
    {
        if (!commenIsTime(&_reader_model->last_modified) || !commenIsTime(&_reader_model->created))
        {
            ERROR("Reader: Time failed!!! %s,  %s\n",asctime(&_reader_model->last_modified),asctime(&_reader_model->created));
        }
    }

    
    return (GObject*)_reader_model;
}

const GObject *readerReadSolid(IndexType index, ReaderScanner* scanner)
{
    _reader_solid = NULL;
    if (scanner)
    {
        char*name = readFromScanner(scanner);
        _reader_solid = modelstoreCreateSolid(_reader_model,commenStripString(name));
        if (_reader_solid)
        {
            _reader_solid->index = index;

            char *keyword = readFromScanner(scanner);
            if (KWORD_AT == kwordType(keyword))
            {
                char *position = readFromScanner(scanner);
                
                commenVertexFromString(position,_reader_solid->p,NULL);
                
                keyword = readFromScanner(scanner);
            }

            _reader_solid->flags = commenStringToULong(keyword) | GM_GOBJECT_FLAG_LOADED;

            keyword = readFromScanner(scanner);
            if (KWORD_COMMENT == kwordType(keyword))
            {
                char commentBuffer[3000];
                commentBuffer[0] = 0;
                while(1)
                {
                    char *comment_line = readFromScanner(scanner);
                    
                    if (NULL == comment_line)
                    {
                        break;
                    }
                    
                    int type = kwordType(comment_line);
                    if (KWORD_COMMENT_END == type)
                    {
                        break;
                    }
                    
                    commenStringCat(commentBuffer,comment_line,sizeof(commentBuffer));
                }
                if (_reader_solid->comments)
                {
                    memory_free(_reader_solid->comments);
                }
                _reader_solid->comments = memory_strdup(commentBuffer);
            }
            else
            {
                pushBackToScanner(scanner,keyword);
            }
        }
    }
    
    return (GObject*)_reader_solid;
}


const GObject *readerReadMaterial(IndexType index, ReaderScanner* scanner)
{
    Material* _material = NULL;
    if (_reader_solid && scanner)
    {
        char*name = readFromScanner(scanner);
        _material = modelstoreCreateMaterial((CObject*)_reader_solid,commenStripString(name));
        
        if (_material)
        {
            _material->index = index;
            
            char *keyword = readFromScanner(scanner);
            _material->min_density = commenStringToDouble(keyword);

            keyword = readFromScanner(scanner);
            _material->color = commenStringToULong(keyword);

            keyword = readFromScanner(scanner);
            _material->min_thick = commenStringToDouble(keyword);
        }
    }
    
    return (GObject*)_material;
}

const GObject *readerReadPolygon(IndexType index, ReaderScanner* scanner)
{
    _reader_polygon = NULL;               
    if (_reader_solid && scanner)
    {
        /*
        char textbuffer[30],textbuffer2[30];
        if (_poly && _poly->first)
        {
            fprintf(stderr,"\"%s\" loaded: x1 = %s, p = %s\n",tok,
                    EPoint3ToString(_poly->first->x,textbuffer,sizeof(textbuffer)),
                    EPoint3ToString(_poly->p,textbuffer2,sizeof(textbuffer2)));
        }
*/
        _reader_polygon = modelstoreCreateOriginalPolygon(_reader_solid);
        if (_reader_polygon)
        {
            _reader_polygon->index = index;
            char *keyword = readFromScanner(scanner);
            if (KWORD_AT == kwordType(keyword))
            {
                unsigned long long flags = 0; // just dummy flags
                char *position = readFromScanner(scanner);
            
                commenVertexFromString(position,_reader_polygon->p,&flags);                
            }
            
            // Fix to read old files too without flags
            IndexType index = 0;
            keyword = readFromScanner(scanner);
            if (0 == objectType(keyword,&index))
            {
                // Consume flags
                _reader_polygon->flags |= commenStringToULongLong(keyword);
            }
            else
            {
                // No flags in file. Leave the symbol to the scanner
                pushBackToScanner(scanner,keyword);
            }
        }
    }
    
    return (GObject*)_reader_polygon;
}

const GObject *readerReadVertex(IndexType index, ReaderScanner* scanner)
{
    if (_reader_polygon && scanner)
    {
        unsigned long long flags = 0;
                        
        char *x = readFromScanner(scanner);
        if (x)
        {
            EPoint vx[3];
#ifdef _DEBUG_LOADER
            LOG("Adding vertex \"%s\"\n",x);
#endif            
            commenVertexFromString(x,vx,&flags);
            
            return (GObject*)modelstoreAddToPolygonOriginal(_reader_polygon, vx, flags, index);
        }
    }
    
    ERROR1("Failed to read vertex\n");
    
    return (GObject*)NULL;
}

int readerConnectionFromString(char* param, IndexType *poly1, IndexType*vert1, IndexType* poly2, IndexType* vert2)
{
    char *p1 = param;
    char *p2 = NULL;
    char *x1 = NULL;
    char *x2 = NULL;
    
    for (char *i = param; *i; i++)
    {
        if (('(' == *i) && ((NULL == p1) || (param == p1)))
        {            
            p1 = i + 1;
        }        

        if ((',' == *i) && (NULL == x1))
        {            
            x1 = i + 1;
            *i = 0;
        }        
        
        if ((',' == *i) && (NULL == p2))
        {
            p2 = i + 1;
            *i = 0;
        }          

        if ((',' == *i) && (NULL == x2))
        {
            x2 = i + 1;
            *i = 0;
        }          
        
        if (')' == *i)
        {
            *i = 0;
        }
    }
    
    *poly1 = commenStringToULong(p1);
    *vert1 = commenStringToULong(x1);
    *poly2 = commenStringToULong(p2);
    *vert2 = commenStringToULong(x2);
    
    return 0;
}

const GObject *readerReadMeasurement(IndexType index, ReaderScanner* scanner)
{
    // "Mea%i(%s):connect(Poly%i,V%i,Poly%i,V%i);unit(%s)\n",
    Mea *mea = NULL;
    if (scanner && _reader_solid)
    {
       char namebuffer[sizeof(mea->name)]; 
       char*name = readFromScanner(scanner);
       strncpy(namebuffer,commenStripString(name),sizeof(namebuffer));
       
        if (KWORD_CONNECT == kwordType(readFromScanner(scanner)))
        {
            char *connection = readFromScanner(scanner);
            IndexType poly1;
            IndexType vert1;
            IndexType poly2;
            IndexType vert2;
            
            readerConnectionFromString(connection, & poly1, &vert1, &poly2, &vert2);

            MeaUnit unit = MeaUnit_None;
            if (KWORD_UNIT == kwordType(readFromScanner(scanner)))
            {
                char *sunit = readFromScanner(scanner);
                unit = meaFindUnit(commenStripString(sunit));
            }        
            
            mea = meaConnect(_reader_solid,
                             namebuffer, 
                             poly1, vert1, 
                             poly2, vert2,0, unit);

            
            if (mea && (0 == mea->index))
            {
                mea->index = index;
            }
        }      
    }
    
    return (GObject*)mea;
}
