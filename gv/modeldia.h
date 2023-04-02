#ifndef __MODELDIA__
#define __MODELDIA__

/* ModelDialog for GV 09.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


void initModelDialog();

int modelEdit(CObject *object, DialogUserChoice callback);
int modelEditClear();


#endif