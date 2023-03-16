#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Exporter for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "exporter.h"
#include "pdfprinter.h"
#include "configuration.h"

typedef int (*PrintModel)(Model *model, const char* fileName);

typedef const char* (*GetPDFNameFromModel)(const char* fileName, char* buffer, int bufferSize);

typedef int (*GetVertexCount)();

typedef struct ExporterT {    
    char name[16];
    
    PrintModel export;
    GetPDFNameFromModel getFilename;
    GetVertexCount getCounter;
} Exporter;

Exporter _exporter[5];
const char *exporter_names[5] = { _exporter[0].name, 
                                  _exporter[1].name, 
                                  _exporter[2].name, 
                                  _exporter[3].name,
                                  _exporter[4].name };

static CommonErrorHandler exporter_error_handler = NULL;

const char** exporterListExporter()
{
    return exporter_names;
}

void exporterPrintError(int err, const char* text, ...)
{
    char textBuffer[256];
    textBuffer[0] = 0;
    
    va_list param;
    va_start(param, text);    
    vsnprintf(textBuffer, sizeof(textBuffer), text, param);    
    va_end(param);    
    
    if (exporter_error_handler)
    {
        exporter_error_handler(err,textBuffer);
    }
    else
    {
        fputs(textBuffer, stderr);
    }    
}


int exporterInit(VertexProjection projection, CommonErrorHandler error_handler )
{
    pdfprinterInit(projection, error_handler );
    exporter_error_handler = error_handler;
    
    memset(_exporter,0,sizeof(_exporter));
    
    strcpy(_exporter[0].name,"PDF");
    _exporter[0].export = pdfprinterPrintModel;
    _exporter[0].getFilename = pdfprinterGetPDFNameFromModel;
    _exporter[0].getCounter = pdfprinterGetVertexCount;
    
    return 0;
}

int exporterExport(Exporter *exporter, Model *model, const char* filename)
{
    if (exporter && exporter->export)
    {
        return exporter->export(model,filename);
    }
    
    return -1;
}

int exporterGetVertexCount(Exporter *exporter)
{
    if (exporter && exporter->getCounter)
    {
        return exporter->getCounter();
    }
    
    return -1;    
}

const char* exporterFilename(Exporter *exporter, const char* fileName, char* buffer, int bufferSize)
{
    if (exporter && exporter->getFilename)
    {
        return exporter->getFilename(fileName,buffer,bufferSize);
    }
    
    return NULL;    
}

Exporter *exporterGetExporter(const char* name)
{
    for (unsigned int i = 0; i < sizeof(_exporter)/sizeof(_exporter[0]); i++)
    {
        if (0 == strcasecmp(_exporter[i].name,name))
        {
            return &_exporter[i];
        }
    }

    exporterPrintError(0, "No exporter found for \"%s\"", name);
    
    return NULL;
}
