
#include <stdio.h>
#include <string.h>


#include "texprinter.h"

#define TEX_EXTENSION ".tex"

FILE *_printerFile = NULL;

static int _texprinterPrintSolid(Solid *solid, int solidCount);
static int _texprinterPrintPolygon(Polygon *poly);
const char* printerGetTexNameFromModel(const char* fileName, char* buffer, int bufferSize);



int texprinterPrintModel(Model *model, const char* fileName)
{
    if (NULL == model)
    {
        fprintf(stderr,"No model. Nothing to print!\n");
        return -1;
    }
        
    _printerFile = fopen(fileName,"w");
        
    if (NULL != _printerFile)
    {
        /*
        char timeCreated[100];
        char timeModified[100];
        
        struct tm tc,lm;
        localtime_r(&model->created, &tc);        
        strftime(timeCreated, sizeof(timeCreated), GM_TIME_FORMAT,&tc);

        localtime_r(&model->last_modified, &lm);        
        strftime(timeModified, sizeof(timeModified), GM_TIME_FORMAT,&lm);

        fprintf(_printerFile,"# File created by %s (V:%s) of Georgi Bootbau\n",GM_APPLICATION,GM_VERSION);
        fprintf(_printerFile,"Model(\"%s\"):\nowner(\"%s\");\nat(%g,%g,%g);created(\"%s\");last_modified(\"%s\")\n",
                model->name,model->owner,
                model->p[0],model->p[1],model->p[2],timeCreated,timeModified);
*/
        int solidCount = 0;
        for (Solid *solid = (Solid*)isCObject(OBJ_SOLID,model->first); NULL != solid; solid = (Solid*)isCObject(OBJ_SOLID,solid->next))
        {     
            solidCount++;
            _texprinterPrintSolid(solid,solidCount);
        }
        
        fclose(_printerFile);
    }
    else
    {
        fprintf(stderr,"Failed to open \"%s\"\n",fileName);
        perror("Unable to open file.\n");
    }
    
    _printerFile = NULL;
    
    return 0;
}

static int _texprinterPrintPolygon(Polygon *poly)
{
    int vertex_count = 1;
    for (Vertex *vertex = poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
    {           
        char stringBuffer[256];
        vertexToString(vertex, stringBuffer, sizeof(stringBuffer),0);
        fprintf(_printerFile,"V%i%s",vertex_count,stringBuffer);
        vertex_count++;

        if (NULL != vertex->next)
        {            
            if (0 == (vertex_count % 3))
            {
                // vertex_count  = 0;
                fprintf(_printerFile,";\n");
            }
            else
            {
                fprintf(_printerFile,";");
            }
        }
        else
        {
            fprintf(_printerFile,"\n");
        }            
    }
    
    return 0;
}

static int _texprinterPrintMaterial(Material *material)
{
   fprintf(_printerFile,"Material(\"%s\");%g;%lX;%g\n",material->name,material->min_density,material->color,material->min_thick);
   
   return 0;
}

static int _texprinterPrintSolid(Solid *solid, int solidCount)
{
    if (NULL == solid)
    {
       return -1;
    }

    fprintf(_printerFile,"Solid%i(\"%s\"):at(%g,%g,%g);%llX\n",solidCount,
            solid->name,
            solid->p[0],solid->p[1],solid->p[2],
            solid->flags);
    
    if (solid->material)
    {
        _texprinterPrintMaterial(solid->material);
    }
    
    int polyCount = 0;
    for (Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
    {    
        polyCount++;
        fprintf(_printerFile,"Poly%i:at(%g, %g, %g);",polyCount,poly->p[0],poly->p[1],poly->p[2]);
        _texprinterPrintPolygon(poly);
    }
    
    return 0;
}    

const char* printerGetTexNameFromModel(const char* fileName, char* buffer, int bufferSize)
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
    
    if (postFix && (restBufferSize > (int)sizeof(TEX_EXTENSION)))
    {
        strcpy(postFix,TEX_EXTENSION);
    }
    else
    {
        strcpy(buffer,fileName);
        strcat(buffer,TEX_EXTENSION);
    }
    
    return configFileName;
}
