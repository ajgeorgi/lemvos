
/* Iterator for GV 23.11.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/



#include "iterator.h"



#ifdef _DEBUG
#define CHECKVERTEX(_v) _checkVertex(__FUNCTION__,__LINE__,_v)
#else
#define CHECKVERTEX(_v)
#endif

// mcheck.h ???

#ifdef _DEBUG
static int _checkVertex(const char* str, int line, const Vertex *vertex)
{
    if (vertex)
    {
        int err = 0;
        if (isGObject(OBJ_VERTEX,vertex))
        {            
            if (vertex->refCount <= 0)
            {
                char vstr[GM_VERTEX_BUFFER_SIZE];
                ERROR("%s:%i:Vertex %s with no reference in Solid\n",                        
                        str?str:UNKNOWN_SYMBOL,line,
                        vertexToString(vertex,vstr,sizeof(vstr),GM_V2S_FLAGS_DEBUG)
                       );
                
                err = -1;
            }    
        }
        else
        {
            ERROR("%s:%i:Invalid vertex in solid \n",str?str:UNKNOWN_SYMBOL,line);
            err = -1;
        }
        
        return err;
    }
    
    return 0;
}
#endif


Vertex *vertexIterator(Vertex *vertex, Solid *solid, flag_type flags)
{
    static Polygon *_poly = NULL;
    flags &= ~GM_GOBJECT_FLAG_POLY_ORIGINAL;
    
    Vertex *next = NULL;
    if (NULL == vertex)
    {
        _poly = NULL;
        
        if (solid)
        {
            Polygon *poly1 = NULL;
            for (poly1 = solid->first; 
                 poly1 && (flags != (poly1->flags & flags)); 
                 poly1 = (Polygon*)poly1->next);
                
            if (poly1)
            {
                vertex = poly1->first;
            }
        }
    }

    if (vertex)
    {
        next = (Vertex*)vertex->next;
    
        if (next && (NULL == next->next))
        {        
            if (NULL == _poly)
            {
                _poly = (Polygon*)isGObject(OBJ_POLYGON,vertex->parent);
            }
            else
            {
                _poly = (Polygon*)_poly->next;
            }
            
            for (; 
                _poly && (flags != (_poly->flags & flags)); 
                _poly = (Polygon*)_poly->next);
            
            if (_poly)
            {
                if (flags != (_poly->flags & flags))
                {
                    _poly = NULL;
                }
            }
        }        
    }

    if (NULL == next)
    {
        if (_poly)
        {
            next = _poly->first;
        }
    }

#ifdef _DEBUG    
    if (next)
    {
        CHECKVERTEX(next);
    }
#endif    
    
    return next;
}

const Vertex *vertexConstIterator(const Vertex *vertex, const Solid *solid, flag_type flags)
{
    static const Polygon *_poly = NULL;
    flags &= ~GM_GOBJECT_FLAG_POLY_ORIGINAL;
    
    const Vertex *next = NULL;
    if (NULL == vertex)
    {
        _poly = NULL;
        
        if (solid)
        {
            const Polygon *poly1 = NULL;
            for (poly1 = solid->first; 
                 poly1 && (flags != (poly1->flags & flags)); 
                 poly1 = (Polygon*)poly1->next);
                
            if (poly1)
            {
                vertex = poly1->first;
            }
        }
    }

    if (vertex)
    {
        next = (const Vertex*)vertex->next;
    
        if (next && (NULL == next->next))
        {        
            if (NULL == _poly)
            {
                _poly = (Polygon*)isGObject(OBJ_POLYGON,vertex->parent);
            }
            else
            {
                _poly = (Polygon*)_poly->next;
            }
            
            for (; 
                _poly && (flags != (_poly->flags & flags)); 
                _poly = (Polygon*)_poly->next);
            
            if (_poly)
            {
                if (flags != (_poly->flags & flags))
                {
                    _poly = NULL;
                }
            }
        }        
    }

    if (NULL == next)
    {
        if (_poly)
        {
            next = _poly->first;
        }
    }

#ifdef _DEBUG    
    if (next)
    {
        CHECKVERTEX(next);
    }
#endif    
    
    return next;
}

const Vertex *vertexOriginalIterator(const Vertex *vertex, const Solid *solid, flag_type flags)
{
    static const Polygon *_poly = NULL;
    
    const Vertex *next = NULL;
    if (NULL == vertex)
    {
        _poly = NULL;
        
        if (solid)
        {
            const Polygon *poly1 = NULL;
            for (poly1 = solid->first_original; 
                 poly1 && (flags != (poly1->flags & flags)); 
                 poly1 = (Polygon*)poly1->next);
                
            if (poly1)
            {
                vertex = poly1->first;
            }
        }
    }

    if (vertex)
    {
        next = (Vertex*)vertex->next;
    
        if (next && (NULL == next->next))
        {        
            if (NULL == _poly)
            {
                _poly = (Polygon*)isGObject(OBJ_POLYGON,vertex->parent);
            }
            else
            {
                _poly = (Polygon*)_poly->next;
            }
            
            for (; 
                _poly && (flags != (_poly->flags & flags)); 
                _poly = (Polygon*)_poly->next);
            
            if (_poly)
            {
                if (flags != (_poly->flags & flags))
                {
                    _poly = NULL;
                }
            }
        }        
    }

    if (NULL == next)
    {
        if (_poly)
        {
            next = _poly->first;
        }
    }

#ifdef _DEBUG    
    if (next)
    {
        CHECKVERTEX(next);
    }
#endif    
    
    return next;
}

Solid *vertexSolidIterator(Model *model, Solid *solid, flag_type flags)
{
    CObject *object = (CObject*)solid;
    
    if ((NULL == object) && model)
    {
        object = model->first;
        if (object && (flags == (object->flags & flags)))
        {
            Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
            if (solid)
            {
                return solid;
            }
        }
    }

    if (object)
    {
        object = (CObject*)object->next;
        
        while(object)
        {
            if (flags == (object->flags & flags))
            {
            solid = (Solid*)isCObject(OBJ_SOLID,object);
            if (solid)
            {
                    return solid;               
            }
            }        
            object = (CObject*)object->next;
        }
    }
    
    return NULL;
}

CObject *vertexCObjectIterator(Model *model, CObject *object, flag_type flags)
{
    if ((NULL == object) && model)
    {
        object = model->first;
        if (object && (flags == (object->flags & flags)))
        {
           return object;
        }
    }

    if (object)
    {
        object = (CObject*)object->next;
        
        while(object)
        {
            if (flags == (object->flags & flags))
            {
                return object;               
            }        
            object = (CObject*)object->next;
        }
    }
    
    return NULL;
}

const CObject *vertexConstCObjectIterator(const Model *model, const CObject *object, flag_type flags)
{
    if ((NULL == object) && model)
    {
        object = model->first;
        if (object && (flags == (object->flags & flags)))
        {
           return object;
        }
    }

    if (object)
    {
        object = (const CObject*)object->next;
        
        while(object)
        {
            if (flags == (object->flags & flags))
            {
                return object;               
            }        
            object = (const CObject*)object->next;
        }
    }
    
    return NULL;
}


const Vertex *vertexConstPolygonIterator(const Polygon *poly, const Vertex *vertex)
{
    if (NULL == vertex)
    {
        if (poly)
        {
            vertex = poly->first;
        }
    }
    else
    {
        vertex = (const Vertex*)vertex->next;
    }
    
#ifdef _DEBUG    
    if (vertex)
    {
        CHECKVERTEX(vertex);
    }
#endif    
    
    return vertex;
}

Vertex *vertexPolygonIterator(Polygon *poly, Vertex *vertex)
{
    if (GM_GOBJECT_FLAG_POLY_ORIGINAL & poly->flags)
    {
        return NULL;
    }
    
    if (NULL == vertex)
    {
        if (poly)
        {
            vertex = poly->first;
        }
    }
    else
    {
        vertex = (Vertex*)vertex->next;
    }
    
#ifdef _DEBUG    
    if (vertex)
    {
        CHECKVERTEX(vertex);
    }
#endif    
    
    return vertex;
}

Polygon *vertexSolidPolygonIterator(Solid *solid, Polygon *poly)
{
    if (NULL == poly)
    {
        if (solid)
        {
            poly = solid->first;
        }
    }
    else
    {
        poly = (Polygon*)poly->next;
    }
    
   return poly;    
}

const Polygon *vertexSolidOriginalPolygonIterator(Solid *solid, const Polygon *poly)
{
    if (NULL == poly)
    {
        if (solid)
        {
            poly = solid->first_original;
        }
    }
    else
    {
        poly = (Polygon*)poly->next;
    }
    
   return poly;        
}

// Polygon *vertexSolidPolygonIterator(Solid *solid, Polygon *poly)
// {
//     if (NULL == poly)
//     {
//         if (solid)
//         {
//             poly = solid->first;
//         }
//     }
//     else
//     {
//         poly = (Polygon*)poly->next;
//     }
//     
//    return poly;    
// }
// 



// const Vertex *vertexConstIter(const CObject *object, const Vertex *vertex, flag_type mask)
// {
//     if (object && object->methods.citerate)
//     {
//         return object->methods.citerate(object,vertex,mask);
//     }
//     char s1[GM_VERTEX_BUFFER_SIZE];
//     ERROR("No iterator for [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
//     
//     return NULL;
// }
// 
// Vertex *vertexIter(CObject *object, Vertex *vertex, flag_type mask)
// {
//     if (object && object->methods.iterate)
//     {
//         return object->methods.iterate(object,vertex,mask);
//     }
// 
//     char s1[GM_VERTEX_BUFFER_SIZE];
//     ERROR("No iterator for [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
// 
//     return NULL;
// }
