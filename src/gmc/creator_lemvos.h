#ifndef __CREATORLEMVOS__
#define __CREATORLEMVOS__

/* Model creator for lemvos for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

// Flag marks polygones to be frames and solids to be hulls
#define LEMVOS_FLAG  GM_FLAG_USER8
#define LEMVOS_FLAG_BOW  GM_FLAG_USER7
#define LEMVOS_FLAG_STERN  GM_FLAG_USER6

#define LEMVOS_MODEL_NAME "lemvos"
#define LEMVOS_HULL_SOLID_NAME "Hull"

int initLemvos(CObject *object);

Model *createLemvos();

#endif // __CREATORLEMVOS__
