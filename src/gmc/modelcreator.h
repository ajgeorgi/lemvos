#ifndef __MODELCREATOR__
#define __MODELCREATOR__

/* Modelcreator for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

Model *modelcreatorCreate(const char *name);

typedef Model* (*CreateModelT)(void);
typedef int (*InitT)(CObject *object);
typedef int (*CalcT)(CObject *object);

typedef void (*ProgressT)(const char *name, int max, int pos);

int modelcreatorAddModel(const char *name, CreateModelT creator, InitT init);

int modelcreatorRunSolidCalculator(const char* model, const char *name, CObject *solid);
int modelcreatorAddSolidCalculator(const char* model, const char *name, CalcT calculator);
char** modelcreatorGetSolidCalculators(const char* model, int *length);

int modelcreatorAddProgressToCalculator(const char* model, const char *name, ProgressT progress);

void modelcreatorProgress(int max, int pos);

int modelcreatorInitModel(Model *model);


#endif // __MODELCREATOR__