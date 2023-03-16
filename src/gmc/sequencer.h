#ifndef __SEQUENCER__
#define __SEQUENCER__

/* Sequencer for GMC 20.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "gobject.h"
#include "mea.h"

typedef CObject* (*CalculationFunction)(const char *name, CObject *object, unsigned long color);

typedef unsigned long (*GetIndexedColor)(int color, int index);

#define GSEQ_FLAG_DISABLE (1<<0)
#define GSEQ_FLAG_INIT    (1<<1)
#define GSEQ_FLAG_FINAL   (1<<2)

typedef struct CalculationT {
    unsigned int flags;
    char name[MODEL_NAME_SIZE];
    CalculationFunction function;    
    int color_index;
} Calculation;

int sequencerInit(GetIndexedColor visualization);

int sequencerAddCalculation(const char*name, CalculationFunction calc, int color_index);
int sequencerAddInitializer(const char*name, CalculationFunction calc, int color_index);
int sequencerAddFinalizer(const char*name, CalculationFunction calc, int color_index);

int sequencerRun();
int sequencerRunObject(CObject *object);

int sequencerEnableCalculation(const char*name, int enable);

Calculation *sequencerGetCalculation(int index);

#endif // __SEQUENCER__