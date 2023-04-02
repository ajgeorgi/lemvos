#ifndef __LOADER__
#define __LOADER__

#include <time.h>

/* Loader for GM 19.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "commen.h"
#include "vertex.h"

typedef int (*LoaderProgress)(int max, int pos);

void initLoader(CommonErrorHandler error_handler);

int loaderLoadModel(Model **model, const char *fileName);
int loaderLoadModelProgress(Model **model, const char *fileName, LoaderProgress progress, int progressCount);

int loaderSaveModel(Model *model, const char *fileName);

const char* loaderGetConfigNameFromModel(const char* fileName, char* buffer, int bufferSize);
int loaderLoadConfig(const char *fileName);

void loaderPrintError(int err, const char* text, ...);


#endif // __LOADER__