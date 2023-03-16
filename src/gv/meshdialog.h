#ifndef __MESHDIALOG__
#define __MESHDIALOG__

/* MeshDialog for GV 09.11.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


void initMeshDialog();

int meshEdit(CObject *object, DialogUserChoice callback);
int meshEditClear();


#endif