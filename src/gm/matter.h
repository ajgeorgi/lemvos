#ifndef __MATTER__
#define __MATTER__

/* Matter for GV 23.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "gobject.h"

typedef struct BlockT {
    // Position of each block is determined by its index i³ of blocks[width,height,length]
    double dot_distance; // Default is 100 mm
    unsigned long long block[16];
    struct BlockT *sub_blocks[1024];
} Block;

typedef struct MatterT {
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


    int width;
    int height;
    int length;
    
    Block *blocks;
    
} Matter;

Matter *createMatter(GObject *parent);
void matterDestroy(Matter *matter);


#endif