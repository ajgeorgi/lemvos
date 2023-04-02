#ifndef __PLOTTER__
#define __PLOTTER__

/* Plotter for GV 26.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


void plotterInit();

int plotterAddModel(Model *model, const char* filename);
int plotterLoadModelFromFile(const char* filename);
int plotterDrawPoint(const EPoint *p, unsigned long color, int size);

int plotterDrawText(int x, int y, const char* text,  unsigned long color);

void plotterHandleError(int code, const char *text);

const GObject **plotterGetSelection(int *length);

void plotterLockSelection(int lock);

#endif // __PLOTTER__