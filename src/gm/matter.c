#include <stdlib.h>
#include <string.h>

/* Matter for GV 23.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "matter.h"
#include "vertex.h"

static int paintMatter(ViewerDriver *driver, GObject *object);


Matter *createMatter(GObject *parent)
{
    Matter *matter = malloc(sizeof(Matter));
    memset(matter,0,sizeof(Matter));
    objectInit((GObject*)matter, parent, 0, OBJ_MATTER);    

    matter->methods.paint = paintMatter;
  
    gobjectClearBoundingBox(&matter->box);    
    
    return matter;
}

void matterDestroy(Matter *matter)
{
    if (isGObject(OBJ_MATTER,(GObject*)matter))
    {
        matter->refCount--;
        if (0 >= matter->refCount)
        {
            if (matter->material)
            {
                vertexDestroyMaterial(matter->material);
            }
            
            matter->magic = 0;
            free(matter);
        }
    }
}

int paintMatter(ViewerDriver *driver, GObject *object)
{
    (void)driver;
    (void)object;
    return -1;
}
