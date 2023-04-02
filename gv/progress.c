
/* Progress for GV 27.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "progress.h"

#include "controller.h"
#include "viewer.h"

#define BUTTON_HEIGHT 20

ProgressBar * progressCreate(ViewerWidget *parent)
{
    (void)parent;
    return NULL;
}

int progressSetProgress(int max, int pos)
{
    char text[100];
    
    snprintf(text,sizeof(text),"%i/%i = %i%%",pos,max,pos/max*100);
    
    plotterDrawText(10, BUTTON_HEIGHT + 10, text, GVCOLOR_WHITE);    
    
    return 0;
}

void progressClose()
{
    viewerClearRectangle(10, BUTTON_HEIGHT + 10, 400, BUTTON_HEIGHT);

    plotterDrawText(10, BUTTON_HEIGHT + 10, NULL, GVCOLOR_WHITE);    
}

