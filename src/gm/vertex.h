#ifndef _VERTEX_
#define _VERTEX_

/* Vertex for GM 19.08.2022
   
   Copyright: Andrej Georgi -> axgeorgi@gmail.com  

   Coordinate system for Models:
    
   Y=v[1]|   /Z=v[2] (BLUE)
   (RED) |  /
         | /
         |/     X=v[0] (GREEN)
         O________

    X = v[X_PLANE]; Y = v[Y_PLANE]; Z = v[Z_PLANE]
    
   The projection on screen should be on the X-Y-Plane
    
   p: position for polygon or solid or model
   x,v: a vertex of a polygon
    
*/

#include "gobject.h"


#define X_PLANE 0
#define Y_PLANE  1
#define Z_PLANE 2
#define GM_EARTH_CENTER { 0, 1, 0 }
#define GM_ZERO_VECTOR { 0, 0, 0 }


#define AddEPoints(_r,_p1,_p2) _r[0] = (_p1[0])+(_p2[0]); _r[1] = (_p1[1])+(_p2[1]); _r[2] = (_p1)[2]+(_p2[2])
#define DiffEPoints(_r,_p1,_p2) _r[0] = (_p1[0])-(_p2[0]); _r[1] = (_p1[1])-(_p2[1]); _r[2] = (_p1)[2]-(_p2[2])
#define CopyEPoint(_r,_p1) _r[0] = (_p1[0]); _r[1] = (_p1[1]); _r[2] = (_p1[2])
#define EPointExtend(_r,_d,_x) _r[0] = ((_d)*(_x[0])); _r[1] = ((_d)*(_x[1])); _r[2] = ((_d)*(_x[2]))
#define ZeroEPoint(_p) _p[0] = 0; _p[1] = 0; _p[2] = 0
#define ScalarProduct(_x,_y) (_x[0]*_y[0] + _x[1]*_y[1] + _x[2]*_y[2])
#define vertexMoveAbsPolygon(_poly,_p) CopyEPoint(_poly->p,_p)
#define vertexMoveAbsSolid(_solid,_p) CopyEPoint(_solid->p,_p)

#define vertexMoveRelPolygon(_poly,_p) AddEPoints(_poly->p,_poly->p,_p)
#define vertexMoveRelSolid(_solid,_p) AddEPoints(_solid->p,_solid->p,_p)

#define cobjectSetVisibility(_s,_v) { if (_v)  (_s)->flags &= ~GM_GOBJECT_FLAG_INVISIBLE; \
                                    else  (_s)->flags |= GM_GOBJECT_FLAG_INVISIBLE; }

#define cobjectIsVisible(_s) (0 == ((_s)->flags & GM_GOBJECT_FLAG_INVISIBLE))


#define vertexCompareEPoint(_x, _y, _eps) (ABS((_x)-(_y)) <= _eps)


typedef struct PolygonT Polygon;
typedef struct VertexT Vertex;
typedef struct MeaT Mea;
typedef struct MeshT Mesh;

// DON'T EVER REAORDER ANYTHING HERE!!!
typedef enum MeaUnitT {
    MeaUnit_None = 0,
    MeaUnit_Pixel,
    MeaUnit_Meter,
    MeaUnit_MiliMeter,
    MeaUnit_MicroMeter,
    MeaUnit_Inch,
    MeaUnit_Foot,
    MeaUnit_Newton,
    MeaUnit_KNewton,
    MeaUnit_KiloGram,
    MeaUnit_Gram,
    MeaUnit_Ton,
    MeaUnit_mNewton,
    MeaUnit_SquarePixel,
    MeaUnit_CubicPixel,
    MeaUnit_SquareMeter,
    MeaUnit_CubicMeter,
    MeaUnit_Count,
    MeaUnit_SquareMiliMeter,
    MeaUnit_CubicDeciMeter,
    MeaUnit_KiloPerCubicDeciMeter,
    MeaUnit_KiloPerCubicMiliMeter,
    MeaUnit_KiloPerCubicMeter,
    MeaUnit_KiloSquarePixel,
    MeaUnit_KiloPixel,
    MeaUnit_KiloCubicPixel,

    MeaUnit_Last
} MeaUnit;
// DON'T EVER REAORDER ANYTHING HERE!!!


typedef int (*VertexProjection)(const EPoint *p, int* x);

#define GM_ATTRIB_FLAG_VALID (1<<0)
#define GM_ATTRIB_FLAG_NORMALE_VALID (1<<1)
#define GM_ATTRIB_FLAG_NORMALE_TO_FACE (1<<2)
#define GM_ATTRIB_FLAG_NORMALE_INSIDE (1<<3)

typedef struct AreaAttributesT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------
  // Don't clone! -------------------------------
  int refCount;  
// ---------------------------
    // Has to be the first entry in attributes because of cloning!
  Normale normale;
// ---------------------------
  
  double area1;
  double area2;
  double volume;

  struct VertexT *v1;
  struct VertexT *v2;
  struct VertexT *v3;
  // struct VertexT *v4;
    
} AreaAttributes;

#define vertexGetMaxDensitey(_m) ( ((_m)->flags & GM_GOBJECT_FLAG_MODIFIED) ? (_m)->edited_max_density : (_m)->max_density )
#define vertexGetMinDensitey(_m) ( ((_m)->flags & GM_GOBJECT_FLAG_MODIFIED) ? (_m)->edited_min_density : (_m)->min_density )

#define vertexGetMaxThick(_m) ( ((_m)->flags & GM_GOBJECT_FLAG_MODIFIED) ? (_m)->edited_max_thick : (_m)->max_thick )
#define vertexGetMinThick(_m) ( ((_m)->flags & GM_GOBJECT_FLAG_MODIFIED) ? (_m)->edited_min_thick : (_m)->min_thick )

typedef struct MaterialT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------
  char name[MODEL_NAME_SIZE];
    
  MeaUnit densityUnit;
  double min_density;
  double max_density;
  double edited_min_density;
  double edited_max_density;
  
  MeaUnit thickUnit;
  double min_thick;
  double max_thick;
  double edited_min_thick;
  double edited_max_thick;
  
  color_type color;
  int color_index;
  
  char spec[100];
  int refCount;

} Material;


typedef struct SolidT {
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
    
    Polygon *first;
    Polygon *last;

    // Original data from file. No manipulations here.
    Polygon *first_original;
    Polygon *last_original;
    
    flag_type triang_filter;
} Solid;



typedef struct PolygonT {
    MagicNumber magic;    
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------
    RepresentationType rep_type;

    Vertex *first;
    Vertex *last;

  // ???: relative: each point is plottet relative to its ancestor OR
  //      absolut: each point is plottet relative to its coordinate system?  
  // This problem is for poligones and origines!
  EPoint p[GM_VERTEX_DIMENSION]; // Origin

} Polygon;

typedef struct VertexT {
  // ???: relative: each point is plottet relative to its ancestor OR
  //      absolut: each point is plottet relative to its coordinate system?  
  // This problem is for poligones and origines!
    MagicNumber magic;    
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------
// Don't change this because of cloning
   int refCount;
    
  EPoint x[GM_VERTEX_DIMENSION];
// ---------------------------
  
  struct VertexT **connections;
  int number_of_connections;
  int max_number_of_connections;
  
  AreaAttributes *first_attrib;    
  AreaAttributes *last_attrib;
  
} Vertex;


typedef struct TriangleT {
    MagicNumber magic;    
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------

    Vertex v[3];
    
    Normale normale;
    
    double area;
    double volume;

} Triangle;

typedef struct ModelT {
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

    
  struct CObjectT *first;
  struct CObjectT *last;

  char owner[128];
  
  LTime created;
  LTime last_modified;
    
  flag_type triang_filter;
  
} Model;

typedef struct MeshT {
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

    Vertex *mesh;
    unsigned int number_of_vertices;
    unsigned int max_number_of_vertices;
    unsigned int real_number_of_vertices;
  
    // Original data from file. No manipulations here.
    Vertex *original_mesh;
    unsigned int original_number_of_vertices;
    unsigned int original_max_number_of_vertices;
  
    unsigned long long _iter;
  
} Mesh;


#include "iterator.h"

extern Vertex *createVertex(GObject *parent, EPoint *x);
extern int vertexCloneVertex(Vertex *v, const Vertex *v1);

// extern int vertexIsBoxEmpty(const BoundingBox *box);
extern int vertexIsNull(const EPoint *x, EPointL epsilon);
extern EPoint vertexBoxDistance(const EPoint *x1, const EPoint *x2);
extern EPoint vertexSphereDistance(const EPoint *x1, const EPoint *x2);

extern EPointL vertexNorm(const EPoint *x);
extern int vertexNormalize(EPoint *n, const EPoint *x);

extern EPointL vertexNormL(const EPointL *x);
extern int vertexNormalizeL(EPointL *n, const EPointL *x);

extern EPoint* vertexMakeNorm(EPoint *norm, const EPoint *x1, const EPoint *x2, const EPoint *x3);
extern EPoint* vertexVectorProduct(EPoint *r, const EPoint *x1, const EPoint *x2);
extern EPointL vertexScalarProduct(const EPoint *x1, const EPoint *x2);
extern EPoint *vertexProject(EPoint *result, const EPoint *dir, const EPoint *vector);

extern EPointL* vertexVectorProductL(EPointL *r, const EPointL *x1, const EPointL *x2);
extern EPointL vertexScalarProductL(const EPointL *x1, const EPointL *x2);

extern EPoint* vertexVectorDiff(EPoint *r, const EPoint *x, const EPoint *x2);
extern EPoint* vertexVectorAdd(EPoint *r, const EPoint *x1, const EPoint *x2);
extern EPoint* vertexVectorCopy(EPoint *r, const EPoint *x1);

#define GM_V2S_FLAG_INDEX (1<<0)
#define GM_V2S_FLAG_VECTOR (1<<1)
#define GM_V2S_FLAG_FLAGS (1<<2)
#define GM_V2S_FLAG_FLAGS_VERBOSE (1<<3)
#define GM_V2S_FLAG_CONNECTIONS (1<<4)
#define GM_V2S_FLAG_FILE_FORMAT (1<<5)

#define GM_V2S_FLAGS_FILE (GM_V2S_FLAG_VECTOR|GM_V2S_FLAG_FLAGS|GM_V2S_FLAG_FILE_FORMAT)
#define GM_V2S_FLAGS_DEBUG (GM_V2S_FLAG_VECTOR|GM_V2S_FLAG_FLAGS|GM_V2S_FLAG_INDEX)
#define GM_V2S_FLAGS_DEBUG_INDEX (GM_V2S_FLAG_INDEX|GM_V2S_FLAG_CONNECTIONS)
#define GM_V2S_FLAGS_DEBUG_VERBOSE (GM_V2S_FLAG_VECTOR|GM_V2S_FLAG_FLAGS|GM_V2S_FLAG_INDEX|GM_V2S_FLAG_FLAGS_VERBOSE|GM_V2S_FLAG_CONNECTIONS)
#define GM_V2S_FLAGS_INDEX_ONLY (GM_V2S_FLAG_FLAGS|GM_V2S_FLAG_INDEX)
#define GM_V2S_FLAGS_INDEX_VERBOSE (GM_V2S_FLAG_FLAGS|GM_V2S_FLAG_INDEX|GM_V2S_FLAG_FLAGS_VERBOSE)

#define GM_VERTEX_BUFFER_SIZE 80
const char*vertexToString(const Vertex* x, char* buffer, size_t size, int flags);
const char* vertexPath(const GObject *v, char *buffer, size_t size);
const char* vertexFlagsToString(const GObject *object, char *buffer, size_t size);
const char* vertexJustFlagsToString(flag_type flags, char *buffer, size_t size);

extern const char*EPoint3ToString(const EPoint* x, char* buffer, size_t size);
extern const char*EPointL3ToString(const EPointL* x, char* buffer, size_t size);


// This is for 3D -> EPoint[3]
int vertexSpawnSurface(const EPoint *n, const EPoint *p, EPoint *e1, EPoint *e2);

// This is for 3D -> EPoint[3]
EPointL vertexDet3(const EPoint *x1, const EPoint *x2, const EPoint *x3);

// This is for 2D -> EPoint[2]
EPointL vertexDet2(const EPoint *x1, const EPoint *x2);

Polygon *vertexGetPolygon(Solid *solid, flag_type flag_mask, Polygon *iterator);

int vertexAddConnection(Vertex *v1, Vertex *v2);
void vertexRemoveConnections(Vertex *vertex);
void vertexRemoveConnectionsFromPoly(Polygon *poly);
Vertex *vertexShareParentConnection(const Vertex *v1, const Vertex *v2);
Vertex *vertexIsConnected(Vertex *v1, Vertex *v2);
Vertex *vertexConnectedParent(CObject *object, Vertex *v1, flag_type mask);

int vertexCloneVertex(Vertex *v, const Vertex *v1);

// SOLID
Solid *createSolid(GObject *parent);
// void solidDestroy(CObject *solid);
// int objectAddComment(CObject *object, const char* comment, int size);

// MESH
Mesh *createMesh(GObject *parent, int size);
// void meshDestroy(CObject *mesh);
// int meshExtend(Mesh *mesh, int size);
Vertex* meshAddPoint(Mesh *mesh, EPoint *x);
void meshClear(Mesh *mesh);

// TRIANGLE
Triangle *createTriangle(GObject *parent, EPoint *x1, EPoint *x2, EPoint *x3);
void vertexDestroyTriangle(Triangle *tri);


Model *createModel();

int vertexDestroy(Vertex *vertex);
void vertexDestroyMaterial(Material *material);
int vertexReplaceMaterial(CObject* object, Material *material);

AreaAttributes *vertexCreateAttribute(Vertex *v);
int vertexDestroyAttribute(AreaAttributes *attrib);
AreaAttributes *vertexCloneAttribute(Vertex *v, const AreaAttributes *attrib);

int vertexCountPolygones(const CObject *object, flag_type flag_mask);
int vertexCountVertices(const CObject *object, flag_type flag_mask);
int vertexCountMeasurements(const CObject *object, flag_type flag_mask);

int vertexGetBoxSize(const BoundingBox *box, double *width, double *length, double *height);
int vertexGetCObjectSize(const CObject *object, double *width, double *length, double *height);

// int vertexSolidDistanceMM(Solid *solid, const EPoint *value, EPoint *valueMM);
int vertexAddToBox(BoundingBox *box, const Vertex *v);
int vertexAddToBoundingBox(CObject *cobject, const Vertex *v);
int vertexMakeBBNormal(BoundingBox *box);
int vertexRotateBB(CObject *object);

int vertexLineIntersection(EPoint *x, const EPoint *dir1, const EPoint *p1, const EPoint *dir2, const EPoint *p2);
int vertexLinePlaneIntersection(EPoint *x, const EPoint *plane_p, const EPoint *plane_n, const EPoint *line_p, const EPoint *line_d) ;
int vertexRotateVertex(CObject *object, const EPoint *v1, EPoint *v2);

int vertexReplaceConnections(CObject *object, Vertex* replace, Vertex *vertex, flag_type mask);
int vertexMergeVertices(CObject *object, Vertex *v1, Vertex *v2);

Model *vertexGetModel(CObject *object);

#endif /* _VERTEX_ */
