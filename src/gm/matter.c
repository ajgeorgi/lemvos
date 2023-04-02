#include <stdlib.h>
#include <string.h>

/* Component for GV 23.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "matter.h"
#include "mea.h"

static int paintComponent(ViewerDriver *driver, GObject *object);

static GObject *iterateComponent(Iter *iter);
static const GObject *citerateComponent(CIter *iter);

static Iter createIterator(CObject *object, flag_type mask);
static CIter createConstIterator(const CObject *object, flag_type mask);

Component *componentExtend(Component *component, int size);
long componentSize(const CObject *obj);

Component *createComponent(GObject *parent, int size)
{
    Component *component = memory_aloc(sizeof(Component));
    memset(component,0,sizeof(Component));
    if (objectInit((GObject*)component, parent, 0, OBJ_COMPONENT))
    {
        // Too much trouble because the iterator returns triangles instead of vertices.
        component->flags |= GM_GOBJECT_FLAG_DO_NOT_CALCULATE;
        
        component->methods.paint = paintComponent;
        
        component->methods.iterate = iterateComponent;
        component->methods.citerate = citerateComponent;    
        component->methods.creatIterator = createIterator;
        component->methods.creatConstIterator = createConstIterator;
        component->methods.destroy = componentDestroy;
        component->methods.objectSize = componentSize;
            
        gobjectClearBoundingBox(&component->box);    
       
        // Convert triangles to vertices
        return componentExtend(component,size / (sizeof(component->tris[0].v)/sizeof(component->tris[0].v[0])));
    }
    
    return NULL;
}

static Iter createIterator(CObject *object, flag_type mask)
{
    Iter iter;
    memset(&iter,0,sizeof(Iter));
    
    iter.object = object;
    iter.mask = mask & ~GM_GOBJECT_FLAG_POLY_ORIGINAL;
    
    return iter;
}

static CIter createConstIterator(const CObject *object, flag_type mask)
{
    CIter iter;
    memset(&iter,0,sizeof(CIter));
    
    iter.object = object;
    iter.mask = mask;
    
    return iter;
}

long componentSize(const CObject *obj)
{
    const Component * component = (const Component * )isCObject(OBJ_COMPONENT,(const GObject*)obj);
    if (component)
    {
        // Size in vertices
        return component->real_number_of_triangles * (sizeof(component->tris[0].v)/sizeof(component->tris[0].v[0]));
    }            
    
    return 0;
}

GObject *iterateComponent(Iter *iter)
{
    Component *component = (Component*)isCObject(OBJ_COMPONENT,iter->object);
    if (component)
    {
        if ((NULL == iter->s1) || (NULL == iter->s2))
        {
            if (component->tris)
            {
                iter->s1 = (GObject*)&component->tris[0];
                iter->s2 = (GObject*)&component->tris[component->number_of_triangles-1];
            }
            else
            {
                iter->s1 = (GObject*)NULL;
                iter->s2 = (GObject*)NULL;                
            }
        }
        
        if (iter->s1 && (iter->s1 <= iter->s2))
        {
            GObject *next = ((GObject *)iter->s1);
            iter->s1 = (GObject*)(((char *)iter->s1) + sizeof(component->tris[0]));
                        
            return isObject(next);
        }
    }
    
    return NULL;
}


const GObject *citerateComponent(CIter *iter)
{
    const Component *component = (const Component*)isCObject(OBJ_COMPONENT,iter->object);
    if (component)
    {
        if ((NULL == iter->s1) || (NULL == iter->s2))
        {
            if (component->tris)
            {
                iter->s1 = (const GObject*)&component->tris[0];
                iter->s2 = (const GObject*)&component->tris[component->number_of_triangles-1];
            }
            else
            {
                iter->s1 = (const GObject*)NULL;
                iter->s2 = (const GObject*)NULL;                
            }
        }
        
        if (iter->s1 && (iter->s1 <= iter->s2))
        {
            const GObject *next = ((const GObject *)iter->s1);
            iter->s1 = (GObject*)(((const char *)iter->s1) + sizeof(component->tris[0]));
                        
            return isObject(next);
        }
    }
    
    return NULL;
}


Component *componentExtend(Component *component, int size)
{
    if (isCObject(OBJ_COMPONENT,(GObject*)component) && (0 < size))
    {    
        char s1[GM_VERTEX_BUFFER_SIZE];
        
        unsigned int old_size = component->max_number_of_triangles;
        component->max_number_of_triangles = component->max_number_of_triangles + size;
        component->tris = (Triangle*)memory_realloc(component->tris, component->max_number_of_triangles * sizeof(Triangle));

        if (component->tris)
        {
            memset(&component->tris[old_size],0,sizeof(Triangle) * size);
            
            int count = 0;
            for (Triangle *t = &component->tris[old_size]; 
                t <= &component->tris[component->max_number_of_triangles-1];
                t++)
            {
                if (objectInit((GObject*)t, (GObject*)component,/* index */  0, OBJ_TRIANGLE))
                {    
                    for (int i = 0; i < (int)(sizeof(t->v)/sizeof(t->v[0]));i++)
                    {
                        objectInit((GObject*)&t->v[i], (GObject*)t,/* index */  0, OBJ_VERTEX);
                        // static memory not dynamic. Carefull when we delete
                        t->v[i].type |= _SOBJ_MARKER;
                    }
                    
                    // static memory not dynamic. Carefull when we delete
                    t->type |= _SOBJ_MARKER;
                    count++;
                }
            }   
            
            LOG("[%s] extended %i->%i by %i triangles (%i vertices)\n",
                vertexPath((GObject*)component,s1,sizeof(s1)),
                old_size,component->max_number_of_triangles,count,count*3);
            
            return component;
        }
        
        FATAL("Could mot extend [%s] by %i triangles to %i\n",
              vertexPath((GObject*)component,s1,sizeof(s1)),
              size,component->max_number_of_triangles);
    }    
    
    return NULL;
}

void componentDestroy(CObject *object)
{
    Component *component = (Component *)isCObject(OBJ_COMPONENT,(GObject*)object);
    if (component)
    {        
        component->refCount--;
        if (0 >= component->refCount)
        {
            destroyAllMea((CObject*)component);
            
            for (unsigned int i = 0; i < component->max_number_of_triangles;i++)
            {
                Triangle *t = &component->tris[i];
                
#ifdef _DEBUG_DELETING                
                char s1[GM_VERTEX_BUFFER_SIZE];

                LOG("Deleting triangle #%i [%s]\n",i,vertexPath((GObject*)t,s1,sizeof(s1)));
#endif                
                // This is a special case and vertexDestroyTriangle() can handle it :-)!                                
                vertexDestroyTriangle(t);
            }
            
            if (component->tris)
            {
                memory_free(component->tris);
                component->tris = NULL;
            }
            if (component->material)
            {
                vertexDestroyMaterial(component->material);
                component->material = NULL;
            }
            
            component->magic = 0;
            
            if (component->comments)
            {
                memory_free(component->comments);
                component->comments = NULL;
            }            
            
            memory_free(component);
        }
    }        
}

Triangle *componentAddTriangle(Component *component, EPoint *x1, EPoint *x2, EPoint *x3)
{
    if (component)
    {      
        if (component->number_of_triangles >= component->max_number_of_triangles)
        {
            componentExtend(component,500);
        }

        Triangle *tri= &component->tris[component->number_of_triangles++];

        tri->flags &= ~GM_GOBJECT_FLAG_INVISIBLE;
        
        if (x1)
        {
            CopyEPoint(tri->v[0].x,x1);
            component->real_number_of_triangles++;
            vertexAddToBoundingBox((CObject*)component,&tri->v[0]);
        }

        if (x2)
        {
            CopyEPoint(tri->v[1].x,x2);
            component->real_number_of_triangles++;
            vertexAddToBoundingBox((CObject*)component,&tri->v[1]);
        }

        if (x3)
        {
            CopyEPoint(tri->v[2].x,x3);
            component->real_number_of_triangles++;
            vertexAddToBoundingBox((CObject*)component,&tri->v[2]);
        }

        component->real_number_of_triangles++;
        
        return tri;
    }
    
    return NULL;        
}

int paintComponent(ViewerDriver *driver, GObject *object)
{
    (void)driver;
    (void)object;
    
    Component *component = (Component*)isCObject(OBJ_COMPONENT,object);
    if (component && driver && driver->drawDot)
    {
        if (0 == (component->flags & GM_GOBJECT_FLAG_INVISIBLE))
        {
            unsigned long triangulation_color = 0;
            
            unsigned long color = driver->getColorFor(GAPERAL_MESH);

            if (component->material)
            {
                color = component->material->color;
            }
            
            if (component->flags & GM_FLAG_SELECTED)
            {
                color = driver->getColorFor(GAPERAL_MESH_SELECTED);
            }
            
            if (GM_GOBJECT_FLAG_SHOW_TRI & component->flags)
            {
                triangulation_color = driver->getColorFor(GAPERAL_TRI);
            }
            
            for (unsigned  i = 0; i < component->number_of_triangles; i++)
            {
                Triangle *tri = &component->tris[i];
                
                if (0 == (component->flags & GM_GOBJECT_FLAG_INVISIBLE))
                {
                    if (tri && (GM_GOBJECT_FLAG_SHOW_TRI & component->flags))
                    {    
                        if (tri->flags & GM_GOBJECT_VERTEX_FLAG_MARK_ERROR)                        
                        {
                            driver->setDrawingColor(driver->getColorFor(GAPERAL_VERTEX_MARKERROR));
                        }
                        else
                        if (tri->flags & GM_GOBJECT_VERTEX_FLAG_MARK)                                                    
                        {
                            driver->setDrawingColor(driver->getColorFor(GAPERAL_VERTEX_MARK));
                        }
                        else
                        if (tri->flags & GM_GOBJECT_VERTEX_FLAG_MARK2)
                        {
                            driver->setDrawingColor(driver->getColorFor(GAPERAL_VERTEX_MARK2));
                        }
                        else                            
                        {
                            driver->setDrawingColor(triangulation_color);
                        }
                        
                        Vertex *vertex = &tri->v[0];
                        for (int i = 1; i < (int)(sizeof(tri->v)/sizeof(tri->v[0]));i++)
                        {
                            Vertex *vert2 = &tri->v[i];

                            if ((vertex->flags & GM_GOBJECT_VERTEX_FLAG_INVALID) && (vert2->flags & GM_GOBJECT_VERTEX_FLAG_INVALID))
                            {
                                driver->setDrawingColor(driver->getColorFor(GAPERAL_ERROR));
                                driver->drawLine(vertex->x, vert2->x);
                                driver->setDrawingColor(triangulation_color);
                            }
                            else
                            {
                                driver->drawLine(vertex->x, vert2->x);
                            }
                            vertex = vert2;
                        }
                    }
                
                    if (GM_GOBJECT_FLAG_SHOW_NORM & component->flags)
                    {
                        if (tri->flags & GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID)
                        {
                            driver->setDrawingColor(driver->getColorFor(GAPERAL_NORMALE));
                            
                            EPoint n[3];
                            const Normale *normale = &tri->normale;
                            n[0] = normale->p[0] + NORM_LENGTH*normale->normal[0];
                            n[1] = normale->p[1] + NORM_LENGTH*normale->normal[1];
                            n[2] = normale->p[2] + NORM_LENGTH*normale->normal[2];
                            driver->drawLine(normale->p, n);                    
                        }
                    }

                    driver->setDrawingColor(color);
                    
                    for (int i = 0; i < (int)(sizeof(tri->v)/sizeof(tri->v[0]));i++)
                    {
                        Vertex *vertex = &tri->v[i];

                        if (GM_GOBJECT_VERTEX_FLAG_MARK_ERROR & vertex->flags)
                        {
                            // This vertex gets a marker
                            driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARKERROR),30);
                            
                            driver->setDrawingColor(color);
                        }
                        else
                        if (GM_GOBJECT_VERTEX_FLAG_MARK & vertex->flags)
                        {
                            // This vertex gets a marker
                            driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARK),25);
                            
                            driver->setDrawingColor(color);
                        }
                        else
                        if (GM_GOBJECT_VERTEX_FLAG_MARK2 & vertex->flags)
                        {
                            // This vertex gets a marker
                            driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARK2),25);
                            
                            driver->setDrawingColor(color);
                        }
                        
                        driver->drawDot(vertex->x);
                    }
                }
            }            
        }
    }
    
    return 0;
}
