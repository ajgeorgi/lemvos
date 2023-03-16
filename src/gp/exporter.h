#ifndef __EXPORTER__
#define __EXPORTER__

/* Exporter for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "commen.h"

typedef struct ExporterT Exporter;

int exporterInit(VertexProjection projection, CommonErrorHandler error_handler);

Exporter *exporterGetExporter(const char* name);

int exporterExport(Exporter *exporter, Model *model, const char* filename);
const char* exporterFilename(Exporter *exporter, const char* fileName, char* buffer, int bufferSize);

int exporterGetVertexCount(Exporter *exporter);

void exporterPrintError(int err, const char* text, ...);

const char** exporterListExporter();

#endif // __EXPORTER__