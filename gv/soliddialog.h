#ifndef __SOLIDDIALOG__
#define __SOLIDDIALOG__

/* SolidDialog for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

void initSolidDialog();

int solidEdit(CObject *solid, DialogUserChoice callback);
int solidEditClear();


#endif