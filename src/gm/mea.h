#ifndef __MEA__
#define __MEA__

/* Mea for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


#define MEA_CONNECTIONS "Connections"
#define MEA_DEBUG "Debug"
#define MEA_DEBUG_COLOR 0xff0000

#define MEA_MAX_NUMBER_OF_VALUES 10

typedef enum ValueTypeT {
    ValueType_None = 0,
    ValueType_Scalar,
    ValueType_Vector,
    ValueType_Connection
} ValueType;

typedef struct ValueT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// --------------------------        
    
    ValueType vtype;
    char name[MODEL_NAME_SIZE];
    char description[127];
    color_type color;
    MeaUnit unit;            
    
    int value_index;
    
    union {
        struct {
            EPointL max;
            EPointL min;
        } scalar;
        struct {
            EPoint vector[GM_VERTEX_DIMENSION];
            EPoint p[GM_VERTEX_DIMENSION];    
        } vector;
        struct {
            EPoint length;
            GObject *x1;
            GObject *x2;      
        } connection;
    } data[MEA_MAX_NUMBER_OF_VALUES];
            
} Value;

#define meaGetScalarValueMax(_v) ((ValueType_Scalar == (_v)->vtype)?(_v)->data[(_v)->value_index].scalar.max:0.0)
#define meaGetScalarValueMin(_v) ((ValueType_Scalar == (_v)->vtype)?(_v)->data[(_v)->value_index].scalar.min:0.0)

#define meaGetVectorValueVector(_v) ((ValueType_Vector == (_v)->vtype)?(_v)->data[(_v)->value_index].vector.vector:GM_ZERO_VECTOR)
#define meaGetVectorValuePosition(_v) ((ValueType_Vector == (_v)->vtype)?(_v)->data[(_v)->value_index].vector.p:GM_ZERO_VECTOR)

#define meaGetValueColor(_v) (((_v)->flags & GM_GOBJECT_FLAG_COLOR) ? (_v)->color : GVCOLOR_WHITE)

typedef struct MeaT {
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
   
    Value *values;
    int max_number_of_values;
    int number_of_values;

    const char* created_by;
} Mea;

Mea *createMesurement(GObject *parent);
int destroyAllMea(CObject *mea);

MeaUnit meaFindUnit(const char* str);
const char* meaUnitToString(MeaUnit unit);

Mea *meaConnect(Solid * solid, const char* name, 
                IndexType poly1index, IndexType v1index, 
                IndexType poly2index, IndexType v2index,
                EPoint length, MeaUnit unit
               );

Mea *meaFind(const CObject *object, const char *name);

Mea* meaGetMea(CObject *object, const char *name);

Value *meaAddScalar(Mea* mea, const char* name, EPointL max, EPointL min, MeaUnit unit);
Value *meaAddVector(Mea* mea, const char* name, const EPoint *p, const EPoint *dir, MeaUnit unit);
Value *meaAddConnection(Mea* mea, const char* name, GObject *v1, GObject *v2, EPoint length, MeaUnit unit);
int meaSetColor(Value *value, color_type color);
int meaGetScalar(const CObject *object, const char* mea_name, const char *value_name, EPoint *max, EPoint *min, MeaUnit *unit);

int meaNumberOfValues(const Mea *mea);
Value *meaFindValue(const Mea* mea, const char* name);

int meaGetConvinientUnit(const Value *value, int index, double *new_value_min, double *new_value_max, MeaUnit *new_unit);

int meaConvert(EPointL value, MeaUnit unit, MeaUnit targetUnit, EPointL *targetValue);

#endif // __MEA__