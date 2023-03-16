#ifndef __PDFPRINTER__
#define __PDFPRINTER__

/* PDFPrinter for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "commen.h"

int pdfprinterInit(VertexProjection projection, CommonErrorHandler error_handler);

int pdfprinterPrintModel(Model *model, const char* fileName);

const char* pdfprinterGetPDFNameFromModel(const char* fileName, char* buffer, int bufferSize);

int pdfprinterGetVertexCount();


#endif