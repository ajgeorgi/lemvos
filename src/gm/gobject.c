
#include <ctype.h>
#define _GNU_SOURCE
#include <string.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* GObject for GM 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define __GOBJECT_PRIVATE
#include "gobject.h"
#undef __GOBJECT_PRIVATE
#include "vertex.h"


Iter _empty_iterator;
CIter _empty_citerator;

typedef struct ObjectsT {
    MagicNumber magic;    
    ObjectType type;
    const char* name;
    int name_length;
} Object;

Object _objects[] = {
    { VMODEL_MAGIC, OBJ_MODEL, "Model" , sizeof("Model")-1 },
    { VSOLID_MAGIC, OBJ_SOLID, "Solid" , sizeof("Solid")-1},
    { VPOLY_MAGIC, OBJ_POLYGON, "Poly" , sizeof("Poly")-1},
    { VMATERIAL_MAGIC, OBJ_MATERIAL, "Material" , sizeof("Material")-1 },
    { VVERTEX_MAGIC, OBJ_VERTEX, "V" , sizeof("V")-1 },
    { VMEA_MAGIC, OBJ_MEASUREMENT, "Calc", sizeof("Calc")-1 },
    { VBOX_MAGIC, OBJ_BOX, "Box", sizeof("Box")-1 },
    { VMESH_MAGIC, OBJ_MESH, "Mesh", sizeof("Mesh")-1 },
    { VWIDGET_MAGIC, OBJ_WIDGET, "Widget", sizeof("Widget")-1 },
    { VTABLE_MAGIC, OBJ_TABLE, "Table", sizeof("Table")-1 },
    { VINPUT_MAGIC, OBJ_INPUT, "Input", sizeof("Input")-1 },
    { VCOMBO_MAGIC, OBJ_COMBO, "Kombo", sizeof("Combo")-1 },
    { VMATTER_MAGIC, OBJ_MATTER, "Qube", sizeof("Qube")-1 },
    { VVALUE_MAGIC, OBJ_VALUE, "Data", sizeof("Data") -1 },
    { VTABSHEET_MAGIC, OBJ_TABSHEET, "Sheet", sizeof("Sheet")-1 },
    { VATTRIB_MAGIC, OBJ_ATTRIB,"Attribut", sizeof("Attribut")-1 },
    { VCONFIG_MAGIC, OBJ_CONFIG, "XFig", sizeof("XFig")-1 }
};


GObject* isObject(const void *obj)
{
    if (obj && _check_memory((void*)obj))
    {
        GObject *gobj = (GObject*)obj;
        ObjectType type = gobj->type & _OBJ_TYPE_MASK;
        if ((0 < type) && (type <= (int)(sizeof(_objects)/sizeof(_objects[0]))))
        {
            Object *_object = &_objects[type-1];
            if (_object->magic == gobj->magic)
            {
                return gobj; // TRUE
            }
        } 
#ifdef _DEBUG      
        // This is not possible because objectInit():117 is using this feature to find reassignments
        // printf("Error: Type %i for Object failed\n",type);
#endif        
    }
    
    return NULL; // FALSE
}

GObject* isGObject(ObjectType type,  const void *obj)
{
    if (obj && _check_memory((void*)obj))
    {
        GObject *gobj = (GObject*)obj;
        type &= ~_WOBJ_MARKER;
        if (type == (type & _OBJ_TYPE_MASK))
        {
            if ((0 < type) && (type <= (int)(sizeof(_objects)/sizeof(_objects[0]))))
            {
                Object *_object = &_objects[type-1];
                if (_object->magic == gobj->magic)
                {
                    return gobj; // TRUE
                }
            } 
        }
#ifdef _DEBUG   
        ObjectType found_type = 0;
        for (int i = 0; i < (int)(sizeof(_objects)/sizeof(_objects[0]));i++)
        {
            if (i + 1 != (_objects[i].type & _OBJ_TYPE_MASK))
            {
                FATAL1("Object type miss definition!\n");
            }
            
            if (0 == _objects[i].magic)
            {
                FATAL1("Object magic not defined!\n");
            }
            
            if (gobj->magic == _objects[i].magic)
            {
                found_type = _objects[i].type & _OBJ_TYPE_MASK;
                break;
            }
        }

        if (0 == found_type)
        {
            FATAL("Error: Type %i for GObject failed. Real type is %i.\n",type & _OBJ_TYPE_MASK, found_type & _OBJ_TYPE_MASK);
        }
        else
        {
            LOG("Warning: Type %i for GObject failed. Real type is %i.\n",type & _OBJ_TYPE_MASK, found_type & _OBJ_TYPE_MASK);
        }
#endif
    }
    
    return NULL; // FALSE    
}

CObject* isCObject(ObjectType type, const void *obj)
{
    if (obj && _check_memory((void*)obj))
    {
        CObject *gobj = ( CObject*)obj;
        ObjectType otype = type & _OBJ_TYPE_MASK;
        if (ANY_COBJECT == type)
        {
            otype = gobj->type & _OBJ_TYPE_MASK;
        }
#ifdef _DEBUG                                    
        ObjectType found_type = 0;
#endif        
        if ((0 < otype) && (otype <= (int)(sizeof(_objects)/sizeof(_objects[0]))))
        {
            Object *_object = &_objects[otype-1];

            if ((_object->magic == gobj->magic) && ((ANY_COBJECT == type) || (_object->type == type)))
            {
                return gobj; // TRUE
            }
        } 
#ifdef _DEBUG  
        for (int i = 0; i < (int)(sizeof(_objects)/sizeof(_objects[0]));i++)
        {
            if (i + 1 != (_objects[i].type & _OBJ_TYPE_MASK))
            {
                FATAL1("Object type miss definition!\n");
            }
            
            if (0 == _objects[i].magic)
            {
                FATAL1("Object magic not defined!\n");
            }            
            
            if (gobj->magic == _objects[i].magic)
            {
                found_type = _objects[i].type & _OBJ_TYPE_MASK;
                break;
            }
        }
        
        if (0 == found_type)
        {
            FATAL("Error: Type %i for CObject failed. Real type is %i.\n",type & _OBJ_TYPE_MASK, found_type & _OBJ_TYPE_MASK);
        }
        else
        {
            LOG("Warning: Type %i for CObject failed. Real type is %i.\n",type & _OBJ_TYPE_MASK, found_type & _OBJ_TYPE_MASK);
        }
        
#endif
    }
    
    return NULL; // FALSE        
}

GObject *objectInit(GObject *obj, GObject *parent, IndexType index, ObjectType type)
{
    if (obj && _check_memory((void*)obj))
    {
        Object *assigned = NULL;
        for (unsigned int i = 0; i < sizeof(_objects)/sizeof(_objects[0]); i++)
        {
            if (_objects[i].type == type)
            {
                assigned  = &_objects[i];
                break;
            }
        }
        
        if (assigned)
        {
            if (isObject(obj))
            {
                ERROR("Reassigning object %i\n",obj->type & _OBJ_TYPE_MASK);
            }
            
            if (obj->type & _COBJ_MARKER)
            {
                memset(obj,0,sizeof(CObject));
            }
            else
            {
                memset(obj,0,sizeof(GObject));
            }

            obj->magic = assigned->magic;            
            obj->type = type;
            obj->parent = parent;
            obj->index = index;
            if (0 == index)
            {
                obj->index = objectGetIndex(obj);
            }
            
            if (obj->type & _COBJ_MARKER)
            {
                CObject *cobj = (CObject *)obj;
                
                objectInit((GObject*)&cobj->box, obj, 0, OBJ_BOX);
                
                gobjectClearBoundingBox(&cobj->box);
            }            
        }
        else
        {
            FATAL("Unknown object of type %i at %p\n",type & _OBJ_TYPE_MASK,obj);
        }
    }
    
    if (! isObject(obj))
    {
        ERROR("Failed to init object type %i at %p\n",type & _OBJ_TYPE_MASK,obj);
        obj = NULL;
    }
    
    return obj;
}

IndexType objectGetIndex(GObject *obj)
{
    if (obj && isObject(obj))
    {
        if (0 < obj->index)
        {
            return obj->index;
        }
        
        if (isObject(obj->parent))
        {
            obj->parent->child_count++;
            obj->index = obj->parent->child_count;
        }
        else
        if (obj->next)
        {
            // Possibly free floating object if no parent and no next
            ERROR1("Error: Index out of order!\n");
        }        
        else
        {
            if (0 == (obj->type & _WOBJ_MARKER)) // No warning for toplevel widgets
            {
                LOG("Warning: Object (%i) without index!\n",obj->type & _OBJ_TYPE_MASK);
            }
        }
        
        return obj->index;
    }
    
    return 0;
}

int objectType(char *obj, IndexType *index)
{
    if (obj)
    {
        for (unsigned int i = 0; i < sizeof(_objects)/sizeof(_objects[0]); i++)
        {
            if (_objects[i].name)
            {
                if (0 == strncmp(obj,_objects[i].name, _objects[i].name_length))
                {
                    if (index)
                    {
                        *index = 0;
                        char *idxstart = NULL;
                        for (char *s = strrchr(obj,'\0') - 1; obj<s; s--)
                        {
                            if (isdigit(*s))
                            {
                                idxstart = s;
                            }
                            else
                            {
                                if (idxstart)
                                {
                                    *index = atol(idxstart);
                                }
                                idxstart = NULL;
                                break;
                            }
                        }

                        if (idxstart)
                        {
                            *index = atol(idxstart);
                        }
                    }
                    return _objects[i].type;
                }
            }
        }
    }
    
#ifdef _DEBUG_LOADER    
    LOG("No object found for %s\n",obj?obj:UNKNOWN_SYMBOL);
#endif
    
    return 0;
}


const char*objectName(ObjectType type)
{
    for (int i = 0; i < (int)(sizeof(_objects)/sizeof(_objects[0]));i++)
    {
        if (type == _objects[i].type)
        {
            return _objects[i].name;
        }
    }
    
    return NULL;
}

ObjectType objectGetType(const GObject *obj)
{
    if (obj && _check_memory((void*)obj))
    {
        for (unsigned int i = 0; i < sizeof(_objects)/sizeof(_objects[0]); i++)
        {
            Object *_object = &_objects[i];

            if (_object->magic == obj->magic)
            {
                return _object->type; 
            }            
        }
    }
    
    return 0;
}

void gobjectClearBoundingBox(BoundingBox *box)
{
    if (isGObject(OBJ_BOX,box))
    {
        // Reset flags except the original flag
        box->flags &= GO_BB_ORIGINAL_DATA;
        
        // Preserve the GObject here
        memset(&box->max,0,sizeof(BoundingBox)-sizeof(GObject));
        
        box->max[0] = -HUGE_VAL;
        box->max[1] = -HUGE_VAL;
        box->max[2] = -HUGE_VAL;
        
        box->min[0] = HUGE_VAL;
        box->min[1] = HUGE_VAL;
        box->min[2] = HUGE_VAL;        
    }
}

int  gobjectIsBoundingBoxCalculated(BoundingBox *box)
{
    if (isGObject(OBJ_BOX,box))
    {
        if (box->flags & GO_BB_CALCULATED)
        {
            return 1;
        }
    }
    
    return 0;
}
