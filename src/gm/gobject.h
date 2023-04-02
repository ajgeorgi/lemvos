#ifndef __GOBJECT__
#define __GOBJECT__

/* GObject for GM 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "commen.h"

#define VMODEL_MAGIC 0xC2AE7FB1
#define VVERTEX_MAGIC 0xB1CF7F21
#define VPOLY_MAGIC 0xA1BFCE01
#define VSOLID_MAGIC 0xA0AFAB01
#define VMATERIAL_MAGIC 0xFFFEAB01
#define VMEA_MAGIC 0xABCEAB81
#define VBOX_MAGIC 0xB07CEA62
#define VWIDGET_MAGIC 0xEFFE01AB
#define VMESH_MAGIC 0xABE5D0F1
#define VTABLE_MAGIC 0xAB1E01F8
#define VINPUT_MAGIC 0x81E90FB2
#define VCOMBO_MAGIC 0xC03B01B5
#define VCOMPONENT_MAGIC 0xA77B0F8E
#define VVALUE_MAGIC  0x7A11BCF3
#define VTABSHEET_MAGIC  0x7AB58EE7
#define VATTRIB_MAGIC 0xA741B07F
#define VCONFIG_MAGIC 0xC0EF1648
#define VTRIANGLE_MAGIC 0x76A467EB

// Complex objects class
#define _COBJ_MARKER (1<<16)
// Widgets class
#define _WOBJ_MARKER (1<<17)
// Data object class
#define _DOBJ_MARKER (1<<18)
// A static object no from dynamic memory
#define _SOBJ_MARKER (1<<19)
// Type identifier mask
#define _OBJ_TYPE_MASK 0x7fff

#define ANY_COBJECT 0
#define ANY_DOBJECT 0

#define OBJ_MODEL (1 | _COBJ_MARKER )
#define OBJ_SOLID (2 | _COBJ_MARKER | _DOBJ_MARKER)
#define OBJ_POLYGON 3
#define OBJ_MATERIAL 4
#define OBJ_VERTEX 5
#define OBJ_MEASUREMENT (6 | _COBJ_MARKER )
#define OBJ_BOX 7
#define OBJ_MESH (8 | _COBJ_MARKER | _DOBJ_MARKER )
#define OBJ_WIDGET (9 | _WOBJ_MARKER)
#define OBJ_TABLE (10 | _WOBJ_MARKER)
#define OBJ_INPUT (11 | _WOBJ_MARKER)
#define OBJ_COMBO (12 | _WOBJ_MARKER)
#define OBJ_COMPONENT (13 | _COBJ_MARKER | _DOBJ_MARKER)
#define OBJ_VALUE 14
#define OBJ_TABSHEET (15 | _WOBJ_MARKER)
#define OBJ_ATTRIB 16
#define OBJ_CONFIG (17 | _WOBJ_MARKER)
#define OBJ_TRIANGLE 18



// flags for CObjects like solid, mesh, model
#define GM_GOBJECT_FLAG_MODIFIED ((flag_type)1<<0)
#define GM_GOBJECT_FLAG_INVISIBLE ((flag_type)1<<1)
#define GM_GOBJECT_FLAG_SHOW_NORM ((flag_type)1<<2)
#define GM_GOBJECT_FLAG_SHOW_TRI ((flag_type)1<<3)
#define GM_GOBJECT_FLAG_SHOW_BBOX ((flag_type)1<<4)
#define GM_GOBJECT_FLAG_SURFACE  ((flag_type)1<<5) // ???
#define GM_GOBJECT_FLAG_TRIANGULATED  ((flag_type)1<<6)
#define GM_GOBJECT_FLAG_DO_NOT_CALCULATE  ((flag_type)1<<7)
#define GM_GOBJECT_FLAG_LOADED  ((flag_type)1<<8)
#define GM_GOBJECT_FLAG_CALCULATED  ((flag_type)1<<9)
#define GM_GOBJECT_FLAG_COLOR  ((flag_type)1<<10)
// polygon flags
#define GM_GOBJECT_FLAG_POLY_ABSOLUT ((flag_type)1<<11)
#define GM_GOBJECT_FLAG_POLY_ORIGINAL ((flag_type)1<<12)
// vertex flags
#define GM_GOBJECT_VERTEX_FLAG_MARK  ((flag_type)1<<13)  // green
#define GM_GOBJECT_VERTEX_FLAG_STRING ((flag_type)1<<14)
#define GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID ((flag_type)1<<15)
#define GM_GOBJECT_VERTEX_FLAG_INVALID ((flag_type)1<<16)
#define GM_GOBJECT_VERTEX_FLAG_MCONNECTED ((flag_type)1<<17)
#define GM_GOBJECT_VERTEX_FLAG_REPORTED ((flag_type)1<<18)
#define GM_GOBJECT_VERTEX_FLAG_MARK2       ((flag_type)1<<19) // white
#define GM_GOBJECT_VERTEX_FLAG_MARK_ERROR  ((flag_type)1<<20) // red
#define GM_GOBJECT_VERTEX_FLAG_SEEN      ((flag_type)1<<21)

#define GM_GOBJECT_VERTEX_FLAG_MASK_MARKER (GM_GOBJECT_VERTEX_FLAG_MARK|GM_GOBJECT_VERTEX_FLAG_MARK2|GM_GOBJECT_VERTEX_FLAG_MARK_ERROR|GM_FLAG_SELECTED)
#define GM_GOBJECT_VERTEX_FLAG_MASK_TEMP (GM_GOBJECT_VERTEX_FLAG_MCONNECTED|GM_GOBJECT_VERTEX_FLAG_REPORTED|GM_GOBJECT_VERTEX_FLAG_SEEN)
#define GM_GOBJECT_VERTEX_FLAG_MASK_STATUS (GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID)


#define GM_GOBJECT_FLAG_MASK_STATUS_FLAGS (GM_GOBJECT_FLAG_LOADED|GM_GOBJECT_FLAG_TRIANGULATED|GM_GOBJECT_FLAG_MODIFIED)
#define GM_GOBJECT_FLAG_MASK_VISIBLE_FLAGS (GM_FLAG_MARK_START_END|GM_GOBJECT_FLAG_DO_NOT_CALCULATE|GM_GOBJECT_FLAG_SHOW_TRI|GM_GOBJECT_FLAG_SHOW_BBOX)

// GM/GV is not using these flags. Some are used in GMC.
#define GM_FLAG_USER1 ((flag_type)1<<24)
#define GM_FLAG_USER2 ((flag_type)1<<25)
#define GM_FLAG_USER3 ((flag_type)1<<26)
#define GM_FLAG_USER4 ((flag_type)1<<27)
#define GM_FLAG_USER5 ((flag_type)1<<28)
#define GM_FLAG_USER6 ((flag_type)1<<29)
#define GM_FLAG_USER7 ((flag_type)1<<30)
#define GM_FLAG_USER8 ((flag_type)1<<31)

#define GM_FLAGS_MASK_USER_FLAGS (GM_FLAG_USER1|GM_FLAG_USER2|GM_FLAG_USER3|GM_FLAG_USER4|GM_FLAG_USER5|GM_FLAG_USER6|GM_FLAG_USER7|GM_FLAG_USER8)

// Flags valid for all objects
#define GM_FLAG_SELECTED ((flag_type)1<<32)
#define GM_FLAG_MARK_START_END ((flag_type)1<<33)


#define GM_FLAG_MASK_NOT_PERSISTENT (GM_GOBJECT_VERTEX_FLAG_MASK_MARKER| \
                                         GM_GOBJECT_VERTEX_FLAG_MASK_TEMP| \
                                         GM_GOBJECT_VERTEX_FLAG_MASK_STATUS| \
                                         GM_GOBJECT_FLAG_MASK_STATUS_FLAGS| \
                                         GM_GOBJECT_FLAG_MASK_VISIBLE_FLAGS)
                                         

typedef unsigned int IndexType;
typedef int ObjectType;
typedef unsigned int MagicNumber;
typedef unsigned long long flag_type;

struct VertexT;
struct GObjectT;
struct CObjectT;
struct MaterialT;
struct MeaT;
struct MeshT;

#define MODEL_NAME_SIZE 64

typedef struct NormaleT {
  EPoint normal[3];
  EPoint surface[3];
  EPoint p[3];     
} Normale;

#ifdef __GOBJECT_PRIVATE
#define GO_BB_CALCULATED (1<<0)
#define GO_BB_NEED_ROTATION (1<<1)
#define GO_BB_ORIGINAL_DATA (1<<2)
#endif

typedef struct BoundingBoxT{    
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// --------------------------        
    
// Don't move these beacuse of box clearing    
    EPoint max[GM_VERTEX_DIMENSION];
    EPoint min[GM_VERTEX_DIMENSION];
// ------------------------------------    
    
    
/*      x5             x6  
 *     /|------------/|
 *  x8/ |         x7/ |
 *   /--|----------/  |
 *   |  |          |  |
 *   |  |----------|--|x3
 *   | / x4        |  /
 *   |/            | /
 * x1/--------------/x2
 */        

    EPoint corner[GM_VERTEX_DIMENSION*8];
    EPoint top_center[GM_VERTEX_DIMENSION];
    
    EPoint center[GM_VERTEX_DIMENSION];
    EPoint p[GM_VERTEX_DIMENSION];
    
    EPointL rotC[GM_VERTEX_DIMENSION];
    EPointL rotS[GM_VERTEX_DIMENSION];
    
    EPointL yaw;
    EPointL roll;
    EPointL pitch;
    
} BoundingBox;


typedef int (*DrawText3D)(EPoint *p, const char* text, size_t length);
typedef int (*DrawLine3D)(const EPoint *p1, const EPoint *p2);
typedef int (*SetDrawingColor)(unsigned long color);
typedef int (*DrawBoundingBox)(BoundingBox *box);
typedef int (*DrawPoint)(const EPoint *p, unsigned long color, int size);
typedef unsigned long (*GetColorFor)(int color);
typedef void (*DrawDot)(const EPoint *x);
typedef void (*DrawTriangle)(const EPoint *x1, const EPoint *x2, const EPoint *x3);

typedef struct ViewerDriverT {
    DrawLine3D drawLine;
    DrawText3D drawText;
    SetDrawingColor setDrawingColor;
    DrawBoundingBox drawBoundingBox;
    DrawPoint drawPoint;
    GetColorFor getColorFor;
    CommonErrorHandler printError;
    DrawDot drawDot; 
    DrawTriangle drawTriangle;
} ViewerDriver;


typedef struct IterT {
    struct CObjectT *object;
    struct GObjectT *s1;
    struct GObjectT *s2;
    flag_type mask;
} Iter;

typedef struct CIterT {
    const struct CObjectT *object;
    const struct GObjectT *s1;
    const struct GObjectT *s2;
    flag_type mask;
} CIter;

extern Iter _empty_iterator;
extern CIter _empty_citerator;

typedef int (*OperateOnCObject)(struct CObjectT *object);
typedef int (*PaintCObject)(ViewerDriver *driver, struct GObjectT *object);
typedef void (*DeleteCObject)(struct CObjectT *object);
typedef struct GObjectT* (*IterateCObject)(Iter* iter);
typedef const struct GObjectT * (*IterateConstCObject)(CIter* iter);
typedef long (*GetObjectSize)(const struct CObjectT *object); // Size in vertices


#define DIALOG_CHOICE_SAVE 1
#define DIALOG_CHOICE_DROP 0

typedef void (*DialogUserChoice)(int choice, struct CObjectT *mesh);
typedef int (*RotateObject)(struct CObjectT *solid, double yaw, double roll, double pitch, double heave);
typedef int (*ObjectEdit)(struct CObjectT *object, DialogUserChoice callback);
typedef int (*RemoveVertex)(struct CObjectT *object, struct GObjectT *o);

typedef Iter (*CreateIterator)(struct CObjectT *object, flag_type mask);
typedef CIter (*CreateConstIterator)(const struct CObjectT *object, flag_type mask);

#define gobjectCreateIterator(_obj,_filter) ((_obj)->methods.creatIterator?(_obj)->methods.creatIterator((CObject*)_obj,_filter): (FATAL1("Not implemented!"), _empty_iterator))
#define gobjectCreateConstIterator(_obj,_filter) ((_obj)->methods.creatConstIterator?(_obj)->methods.creatConstIterator((const CObject*)_obj,_filter): (FATAL1("Not implemented!"), _empty_citerator))

#define gobjectIterate(_iter) (_iter.object->methods.iterate(&_iter))
#define gobjectConstIterate(_citer) (_citer.object->methods.citerate(&_citer))

#define gobjectRemoveObject(_obj,_gobj) ((_obj)->methods.remove?(_obj)->methods.remove(_obj,(GObject*)_gobj): FATAL1("Not implemented!"))

#define gobjectDelete(_obj) ((_obj)->methods.destroy?(_obj)->methods.destroy(_obj): FATAL1("Not implemented!"))
#define gobjectEdit(_obj) ((_obj)->methods.objectEdit?(_obj)->methods.objectEdit(_obj): FATAL1("Not implemented!"))

typedef struct CObjectMethodsT {
    PaintCObject paint;
    OperateOnCObject triangulate;
    OperateOnCObject tetrahedralizat;
    OperateOnCObject calcAreaNormale;
    ObjectEdit objectEdit;
    DeleteCObject destroy;
    IterateCObject iterate;
    IterateConstCObject citerate;
    RotateObject rotate;
    RemoveVertex remove;
    CreateIterator creatIterator;
    CreateConstIterator creatConstIterator;
    GetObjectSize objectSize;
} CObjectMethods;

typedef struct GObjectT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// --------------------------    
} GObject;

typedef struct CObjectT {
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
// --------------------------    
} CObject;

typedef struct DObjectT {
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
// --------------------------    
    BoundingBox original_box;
    Normale normal;    
    struct MeshT *wettet;
// --------------------------    
} DObject;

GObject* isObject(const void *obj);
GObject* isGObject(ObjectType type, const void *obj);
CObject* isCObject(ObjectType type, const void *obj);
DObject* isDObject(ObjectType type, const void *obj);


GObject *objectInit(GObject *obj, GObject *parent, IndexType index, ObjectType type);

ObjectType objectType(char *obj, IndexType *index);
IndexType objectGetIndex(GObject *obj);
const char*objectName(ObjectType type);

ObjectType objectGetType(const GObject *obj);

int gobjectClearBoundingBox(BoundingBox *box);
int  gobjectIsBoundingBoxCalculated(BoundingBox *box);

int objectAddComment(CObject *object, const char* comment, int size);


#endif // __GOBJECT__