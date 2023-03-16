#ifndef __PROGRESS__
#define __PROGRESS__

/* Progress for GV 27.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"

typedef struct ProgressBarT {
    ViewerWidget *widget;
    
    const char* name;
    int max;
    int pos;
} ProgressBar;

ProgressBar * progressCreate(ViewerWidget *parent);

void progressSet(const char *name, int max, int pos);
void progressSetProgress(int pos);


#endif // __PROGRESS__
