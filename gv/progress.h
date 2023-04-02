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

int progressSetProgress(int max, int pos);
void progressClose();


#endif // __PROGRESS__
