#include <string.h>

/* Mea for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "mea.h"
#include "modelstore.h"


static int meaPaint(ViewerDriver *driver, struct GObjectT *object);
static void deleteMeasurement(struct CObjectT *object);

Mea *createMesurement(GObject *parent)
{
    Mea *mea = memory_aloc(sizeof(Mea));
    memset(mea,0,sizeof(Mea));
    objectInit((GObject*)mea, parent, 0, OBJ_MEASUREMENT);    

    mea->methods.paint = meaPaint;
    mea->methods.destroy = deleteMeasurement;
    
    gobjectClearBoundingBox(&mea->box);
    
    CObject *object = isCObject(ANY_COBJECT,parent);
    if (object)
    {
        // Inherit material of parent
        mea->material = object->material;
        if (mea->material)
        {
            mea->material->refCount++;
        }
    }
        
    return mea;
}

void deleteMeasurement(struct CObjectT *object)
{
    Mea *mea = (Mea*)isCObject(OBJ_MEASUREMENT,object);
    if (mea)
    {
        mea->refCount--;
        if (0 >= mea->refCount)
        {
            mea->magic = 0;
            
            if (mea->material)
            {
                vertexDestroyMaterial(mea->material);
            }
            mea->material = NULL;
            
            if (mea->comments)
            {
                memory_free(mea->comments);
            }
            
            if (mea->values)
            {
                memory_free(mea->values);
            }
            
            memory_free(mea);                   
        }
    }
    else
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        ERROR("Trouble deleting measurement on [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
    }
}

int destroyAllMea(CObject *object)
{
    if (isCObject(ANY_COBJECT,object))
    {
        for (CObject* mea = object->first_mea; mea;)
        {
            CObject *next= (CObject* )isObject(mea->next);
            deleteMeasurement(mea);
            mea = next;
        }
        
        object->first_mea = NULL;
        object->last_mea = NULL;
        
        return 0;
    }
    
    return 1;
}


typedef struct UnitTableT {
    const char *name;
    MeaUnit unit;    
} UnitTable;


#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-source-encoding"
#endif

// DON'T EVER REAORDER ANYTHING HERE!!!
static const UnitTable _unitTable[] = {
    { "m" , MeaUnit_Meter },    
    { "mm" , MeaUnit_MiliMeter },
    { "µm" , MeaUnit_MicroMeter },
    { "micro" , MeaUnit_MicroMeter },
    { "mu" , MeaUnit_MicroMeter },
    { "um" , MeaUnit_MicroMeter },
    { "px" , MeaUnit_Pixel },
    { "p" , MeaUnit_Pixel },
    { "in" , MeaUnit_Inch },
    { "N", MeaUnit_Newton },
    { "KN", MeaUnit_KNewton },
    { "Kg", MeaUnit_KiloGram },
    { "g", MeaUnit_Gram },
    { "t", MeaUnit_Ton },  
    { "mN", MeaUnit_mNewton },
    { "px²", MeaUnit_SquarePixel },
    { "px³", MeaUnit_CubicPixel },
    { "m²", MeaUnit_SquareMeter },
    { "m³", MeaUnit_CubicMeter },
    { "#", MeaUnit_Count },
    { "mm²" ,MeaUnit_SquareMiliMeter },
    { "dm³", MeaUnit_CubicDeciMeter },
    { "Kg/dm³", MeaUnit_KiloPerCubicDeciMeter },
    { "Kg/mm³", MeaUnit_KiloPerCubicMiliMeter },
    { "Kg/m³", MeaUnit_KiloPerCubicMeter },
    { "kpx²", MeaUnit_KiloSquarePixel },
    { "kpx", MeaUnit_KiloPixel },
    { "kpx³", MeaUnit_KiloCubicPixel },
};
// DON'T EVER REAORDER ANYTHING HERE!!!

#ifdef __clang__
#pragma GCC diagnostic pop
#endif


MeaUnit meaFindUnit(const char* str)
{
    if (str)
    {
        for (unsigned int i = 0; i < sizeof(_unitTable)/sizeof(_unitTable[0]); i++)
        {
            if (0 == strcasecmp(str,_unitTable[i].name))
            {
                return _unitTable[i].unit;
            }        
        }
    }

    return MeaUnit_None;
}

const char* meaUnitToString(MeaUnit unit)
{
    for (unsigned int i = 0; i < sizeof(_unitTable)/sizeof(_unitTable[0]); i++)
    {
        if (unit == _unitTable[i].unit)
        {
            return _unitTable[i].name;
        }        
    }

    return "?";    
}


Mea *meaConnect(Solid * solid, const char* name, 
                IndexType poly1index, IndexType v1index, 
                IndexType poly2index, IndexType v2index,
                EPoint length, MeaUnit unit
               )
{
    Polygon *poly1 = modelstoreGetPolygon(solid,poly1index);
    Polygon *poly2 = modelstoreGetPolygon(solid,poly2index);
    
    Vertex *v1 = modelstoreGetVertex(poly1,v1index);
    Vertex *v2 = modelstoreGetVertex(poly2,v2index);

    if (v1 && v2)
    {
        Mea *mea = meaFind((CObject*)solid,MEA_CONNECTIONS);
        
        if (NULL == mea)
        {
            mea = modelstoreCreateMeasurement((CObject*)solid,MEA_CONNECTIONS);
        }
        
        // Value *value = NULL;
        if (mea)
        {
            meaAddConnection(mea, name, (GObject*)v1, (GObject*)v2, length, unit);
        }
        
        return mea;
    }
    
    return NULL;
}

int meaPaint(ViewerDriver *driver, struct GObjectT *object)
{
    Mea *mea = (Mea*)isCObject(OBJ_MEASUREMENT,object);
    if (mea && driver && driver->drawLine)
    {
        if (0 == (mea->flags & GM_GOBJECT_FLAG_INVISIBLE))
        {
            color_type color = driver->getColorFor(GAPERAL_MEASUREMENT);
            color_type center_color = driver->getColorFor(GAPERAL_CENTER_POINT);

            if (mea->flags & GM_FLAG_SELECTED)
            {
                color = driver->getColorFor(GAPERAL_MESH_SELECTED);
            }

            for (int i = 0; i < mea->number_of_values; i++)
            {
                color_type ccolor = center_color;
                color_type vcolor = color;
                if (ValueType_Vector == mea->values[i].vtype)
                {
                    Value *value = &mea->values[i];

                    if (value->flags & GM_GOBJECT_FLAG_COLOR)
                    {
                        ccolor = vcolor = value->color;
                    }
                    
                    driver->setDrawingColor(vcolor);

                    EPoint x[3];
                    AddEPoints(x,value->data[value->value_index].vector.p,value->data[value->value_index].vector.vector);
                    
                    driver->drawLine(value->data[value->value_index].vector.p, x);
                    
                    int cross_size = 50;
                    if (GM_GOBJECT_VERTEX_FLAG_MARK & value->flags)
                    {
                        cross_size = 150;
                        ccolor = ccolor | 0xff0000;
                    }
                    
                    driver->drawPoint(value->data[value->value_index].vector.p,ccolor,cross_size);
                }
            }                
        }
    }
    
    return 0;
}

Mea *meaFind(const CObject *object, const char *name)
{
    if (isCObject(ANY_COBJECT,object))
    {
        for (Mea *mea = (Mea*)object->first_mea; mea; mea = (Mea*)mea->next)
        {
            if (0 == strncmp(mea->name,name,sizeof(mea->name)))
            {
                return mea;
            }
        }
    }
    
    return NULL;
}

Value *_meaCreateValue(Mea* mea, ValueType type)
{
    if (isCObject(OBJ_MEASUREMENT,mea))
    {
        if (mea->number_of_values >= mea->max_number_of_values)
        {
            mea->max_number_of_values += 10;
        
            mea->values = (Value*)memory_realloc(mea->values,sizeof(Value)*mea->max_number_of_values);        
        }
        
        Value* value = &mea->values[mea->number_of_values++];
        
        memset(value,0,sizeof(Value));
        
        objectInit((GObject*)value, (GObject*)mea, 0, OBJ_VALUE);    
        
        value->vtype = type;
        
        return value;
    }
    
    return NULL;
}

Value *meaFindValue(const Mea* mea, const char* name)
{
    if (mea)
    {
        for (int i = 0; i < mea->number_of_values; i++)
        {
            if (0 == strncmp(mea->values[i].name,name,sizeof(mea->values[i].name)))
            {
                Value *value = &mea->values[i];
                
                return value;
            }
        }
    }
    
    return NULL;
}

int meaGetConvinientUnit(const Value *value, int index, double *new_value_min, double *new_value_max, MeaUnit *new_unit)
{
    if (value && new_value_max && new_value_min && new_unit)
    {
        MeaUnit unit = MeaUnit_None;
        double value_max  = 0;
        double value_min  = 0;
        
        if (index < 0)
        {
            index = value->value_index;
        }
        
        switch(value->vtype)
        {
            case ValueType_Scalar: {
                value_max  = value->data[index].scalar.max;
                value_min  = value->data[index].scalar.min;
            } break;
            
            case ValueType_Vector: {
                value_max  = vertexNorm(value->data[index].vector.vector);
                value_min  = value_max  ;                
            } break;      
            
            case ValueType_Connection: {
                value_max  = value->data[index].connection.length;
                value_min  = value_max  ;
            } break;

            default:
                ;
        }                    
        
        unit = value->unit;                                
        
        switch(unit)
        {
            case MeaUnit_SquarePixel: {
                
                *new_value_max = value_max  / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                *new_value_min = value_min  / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                
                *new_unit = MeaUnit_SquareMeter;
                
            } break;
            
            case MeaUnit_SquareMeter: {
                if (value_max < 0.1)
                {
                    *new_value_max = value_max  * 10000.0;
                    *new_value_min = value_min  * 10000.0;
                    
                    *new_unit = MeaUnit_SquareMiliMeter;
                }                
            } break;
            
            case MeaUnit_CubicPixel: {
                *new_value_max = value_max  / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                *new_value_min = value_min  / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);

                *new_unit = MeaUnit_CubicMeter;
            } break;
            
            case MeaUnit_Newton: {
                if (value_max > 1000000.0)
                {
                    *new_value_max = value_max  / 1000.0;
                    *new_value_min = value_min  / 1000.0;

                    *new_unit = MeaUnit_KNewton;                    
                }
                if (value_max < 1.0)
                {
                    *new_value_max = value_max  * 1000.0;
                    *new_value_min = value_min  * 1000.0;

                    *new_unit = MeaUnit_mNewton;                    
                }                
            } break;
                        
            case MeaUnit_KNewton: {
                if (value_max < 1.0)
                {
                    *new_value_max = value_max  * 1000.0;
                    *new_value_min = value_min  * 1000.0;

                    *new_unit = MeaUnit_Newton;                    
                }                                
            } break;
            
            default:
                *new_value_max = value_max;
                *new_value_min = value_min;                
                *new_unit = unit;
                return 0; 
        }
        
        return 0;
    }
    
    return -1;
}

int meaSetColor(Value *value, color_type color)
{
    if (isObject(value))
    {
        if (GVCOLOR_TRANSPARENT == color)
        {
            value->color = 0;                                
            value->flags &= ~GM_GOBJECT_FLAG_COLOR;            
        }
        else
        {
            value->color = color;                                
            value->flags |= GM_GOBJECT_FLAG_COLOR;
        }
        return 0;
    }
    
    return 1;
}

Value *meaAddScalar(Mea* mea, const char* name, EPointL max, EPointL min, MeaUnit unit)
{
    if (isObject(mea))
    {
        Value *value = meaFindValue(mea,name);
        if (NULL == value)
        {
            value = _meaCreateValue(mea,ValueType_Scalar);    
            strncpy(value->name,name,sizeof(value->name));
        }
        
        if (value->flags & GM_GOBJECT_FLAG_MODIFIED)
        {
            if (value->value_index < (int)(sizeof(value->data)/sizeof(value->data[0])))
            {
                value->value_index++;
            }
        }
        
        value->data[value->value_index].scalar.min = min;
        value->data[value->value_index].scalar.max = max;
        value->unit = unit;
        
        value->flags |= GM_GOBJECT_FLAG_MODIFIED;
        
        return value;
    }
    
    return NULL;
}

Value *meaAddVector(Mea* mea, const char* name, const EPoint *p, const EPoint *dir, MeaUnit unit)
{
    if (isObject(mea))
    {
        Value *value = meaFindValue(mea,name);
        if (NULL == value)
        {
            value = _meaCreateValue(mea,ValueType_Vector);    
            strncpy(value->name,name,sizeof(value->name));
        }

        if (value->flags & GM_GOBJECT_FLAG_MODIFIED)
        {
            if (value->value_index < (int)(sizeof(value->data)/sizeof(value->data[0])))
            {
                value->value_index++;
            }
        }
        
        if (dir)
        {
            CopyEPoint(value->data[value->value_index].vector.vector,dir);
        }
        if (p)
        {
            CopyEPoint(value->data[value->value_index].vector.p,p);
        }
        value->unit = unit;
        value->flags |= GM_GOBJECT_FLAG_MODIFIED;

        return value;    
    }
    
    return NULL;
}

Value *meaAddConnection(Mea* mea, const char* name, GObject *v1, GObject *v2, EPoint length, MeaUnit unit)
{
    if (isObject(mea))
    {
        Value *value = meaFindValue(mea,name);
        if (NULL == value)
        {
            value = _meaCreateValue(mea,ValueType_Connection);    
            strncpy(value->name,name,sizeof(value->name));
        }
        
        value->data[value->value_index].connection.x1 = v1;
        value->data[value->value_index].connection.x2 = v2;
        value->data[value->value_index].connection.length = length;
        value->unit = unit;
        
        return value;        
    }
    return NULL;
}

int meaNumberOfValues(const Mea *mea)
{
    if (isObject(mea))
    {
        return mea->number_of_values;
    }
    
    return -1;
}


int meaGetScalar(const CObject *object, const char* mea_name, const char *value_name, EPoint *max, EPoint *min, MeaUnit *unit)
{
    if (isObject(object))
    {
        const Mea *mea = meaFind(object, mea_name);

        if (mea)
        {        
            Value *value = meaFindValue(mea, value_name);
            if (value)
            {
                if (ValueType_Scalar == value->vtype)
                {
                    if (min)
                    {
                        *min = value->data[value->value_index].scalar.min;
                    }
                    if (max)
                    {
                        *max = value->data[value->value_index].scalar.max;
                    }
                    if (unit)
                    {
                        *unit = value->unit;
                    }
                    
                    return 0;
                }
            }
        }
        
#ifdef _DEBUG
            char s1[GM_VERTEX_BUFFER_SIZE];
            LOG("Could not find value \"%s\" of \"%s\" from [%s]\n",
                value_name?value_name:UNKNOWN_SYMBOL,
                mea_name?mea_name:UNKNOWN_SYMBOL,
                vertexPath((GObject*)object,s1,sizeof(s1)));
#endif                    
    }
    
    return -1;
}

Mea* meaGetMea(CObject *object, const char *name)
{
    if (isCObject(ANY_COBJECT,object))
    {
        Mea *mea = meaFind(object, name);
        if (NULL == mea)
        {
            mea = modelstoreCreateMeasurement(object,name);
        }
        return mea;
    }
    
    return NULL;
}


int meaConvert(EPointL value, MeaUnit unit, MeaUnit targetUnit, EPointL *targetValue)
{
#define MINP       GMATH_PIXEL_PER_M
#define SQUARE_M   (MINP*MINP)
#define CUBIC_M    (MINP*MINP*MINP)
    
    static const EPointL conversionMatrix[MeaUnit_Last * MeaUnit_Last] = {
    //     ,                  None, Pixel,          Meter,             MiliMeter,       MicroMeter,      Inch,Foot,Newton,KNewton,KiloGram,Gram,Ton,mNewton,SquarePixel,CubicPixel,  SquareMeter,CubicMeter,Count,SquareMiliMeter,CubicDeciMeter,KiloPerCubicDeciMeter,KiloPerCubicMiliMeter,KiloPerCubicMeter
    /* None */                0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Pixel */               0,    1,              MINP,              MINP/1000.0,     MINP/1000000.0,  0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Meter */               0,    1.0/MINP,       1,                 1.0/1000.0,      1.0/1000000.0,   0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* MiliMeter */           0,    1000.0/MINP,    1000,              1.0,             1000.0,          0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* MicroMeter */          0,    1000000.0/MINP, 1000000,           1.0/1000000.0,   1.0,             0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Inch */                0,    0,              25.4,              0.254,           0,               1.0, 0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Foot */                0,    0,              0,                 0,               0,               0,   1.0, 0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Newton */              0,    0,              0,                 0,               0,               0,   0,   1.0,   0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,                  
    /* KNewton */             0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,                  
    /* KiloGram */            0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Gram */                0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* Ton */                 0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* mNewton */             0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0, 
    /* SquarePixel */         0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      1.0,        0,           SQUARE_M,   0,          0,    0,               0,             0,                    0,                    0,
    /* CubicPixel */          0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          1.0,         0,          CUBIC_M,    0,    0,               0,             0,                    0,                    0,
    /* SquareMeter */         0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      1.0/(SQUARE_M),        1.0,          0,          0,    0,               0,             0,                    0,                    0,
    /* CubicMeter */          0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          1.0/CUBIC_M, 0,          1.0,        0,    0,               0,             0,                    0,                    0,
    /* Count */               0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* SquareMiliMeter */     0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* CubicDeciMeter */      0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* KiloPerCubicDeciMeter */0,   0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* KiloPerCubicMiliMeter */0,   0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0,
    /* KiloPerCubicMeter */   0,    0,              0,                 0,               0,               0,   0,   0,     0,      0,       0,   0,  0,      0,          0,           0,          0,          0,    0,               0,             0,                    0,                    0
    };
    
    
    EPointL factor = conversionMatrix[unit + targetUnit *MeaUnit_Last];
    
    // LOG("Last=%i, u=%i, t=%i\n",MeaUnit_Last,unit,targetUnit);
    if (targetValue)
    {
        if (0 != factor)
        {
            // 1 + 2 *last
            *targetValue = value * factor;
#ifdef _DEBUG_MEA            
            LOG("Conversion %Lg%s [%i,%i](%Lg)-> %Lg%s\n",value,meaUnitToString(unit),unit,targetUnit,factor,*targetValue,meaUnitToString(targetUnit));
#endif            
            return 0;
        }
    }
    
    ERROR("Conversion %s->%s failed\n",meaUnitToString(unit),meaUnitToString(targetUnit));
    
    return -1;
}
            
