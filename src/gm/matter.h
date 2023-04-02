#ifndef __COMPONENT__
#define __COMPONENT__

/* Component for GV 23.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "gobject.h"
#include "vertex.h"


typedef struct ComponentT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------
    CObjectMethods methods;    
    char name[MODEL_NAME_SIZE];
    BoundingBox box;
    int refCount;
    struct CObjectT *first_mea;
    struct CObjectT *last_mea;  
    struct MaterialT *material;
    EPoint p[GM_VERTEX_DIMENSION]; // Origin
    char *comments;    
    RepresentationType rep_type;    
// ---------------------------
    BoundingBox original_box;
    Normale normal;    
    struct MeshT *wettet;
// --------------------------    

    Triangle *tris;
    
    unsigned int number_of_triangles;
    unsigned int max_number_of_triangles;  
    unsigned int real_number_of_triangles;
    
} Component;

Component *createComponent(GObject *parent, int size);
void componentDestroy(CObject *component);

Triangle *componentAddTriangle(Component *component, EPoint *x1, EPoint *x2, EPoint *x3);

#endif // __COMPONENT__