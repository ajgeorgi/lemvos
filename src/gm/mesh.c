#include <stdio.h>
#include <math.h>
#include <string.h>

/* Mesh for GM 13.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "mea.h"

static int meshPaint(ViewerDriver *driver, struct GObjectT *object);

static GObject *iterateMesh(Iter *iter);
static const GObject *citerateMesh(CIter *iter);

int meshRemoveVertex(CObject *object, GObject *v);
static Iter createIterator(CObject *object, flag_type mask);
static CIter createConstIterator(const CObject *object, flag_type mask);
static long meshSize(const CObject *obj);
static void meshDestroy(CObject *object);
static int meshExtend(Mesh *mesh, int size);

Mesh *createMesh(GObject *parent, int size)
{
    Mesh *mesh = memory_aloc(sizeof(Mesh));
    if (mesh)
    {
        memset(mesh,0,sizeof(Mesh));
        objectInit((GObject*)mesh, parent, 0, OBJ_MESH);    

        mesh->methods.paint = meshPaint;
        mesh->methods.iterate = iterateMesh;
        mesh->methods.citerate = citerateMesh;    
        mesh->methods.remove = meshRemoveVertex;
        mesh->methods.creatIterator = createIterator;
        mesh->methods.creatConstIterator = createConstIterator;
        mesh->methods.destroy = meshDestroy;
        mesh->methods.objectSize = meshSize;

        meshExtend(mesh,size);
        
        return mesh;
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

void meshDestroy(CObject *object)
{
    Mesh *mesh = (Mesh *)isCObject(OBJ_MESH,(GObject*)object);
    if (mesh)
    {        
        mesh->refCount--;
        if (0 >= mesh->refCount)
        {
            // meshClear(mesh);
            destroyAllMea((CObject*)mesh);
            
            char s1[GM_VERTEX_BUFFER_SIZE];
            for (unsigned int i = 0; i < mesh->max_number_of_vertices;i++)
            {
                Vertex *v = &mesh->mesh[i];
                
                if (1 < v->refCount)
                {                    
                    ERROR("Too many refs on vertex [%s]! Might cause mem holes.\n",
                          vertexPath((GObject*)v,s1,sizeof(s1)));
                }
#ifdef _DEBUG_DELETING                
                LOG("Deleting vertex #%i [%s]\n",i,vertexPath((GObject*)v,s1,sizeof(s1)));
#endif                
                // This is a special case and vertexDestroy() can handle it :-)!                                
                vertexDestroy(v);
            }
            
            if (mesh->mesh)
            {
                memory_free(mesh->mesh);
                mesh->mesh = NULL;
            }
            if (mesh->material)
            {
                vertexDestroyMaterial(mesh->material);
                mesh->material = NULL;
            }
            
            mesh->magic = 0;
            
            if (mesh->comments)
            {
                memory_free(mesh->comments);
                mesh->comments = NULL;
            }            
            
            memory_free(mesh);
        }
    }        
}


void meshClear(Mesh *mesh)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh))
    {
        mesh->flags &= ~GM_GOBJECT_FLAG_MASK_STATUS_FLAGS;
        
        gobjectClearBoundingBox(&mesh->box);
        
        if (mesh->mesh)
        {
            
            for (unsigned int i = 0; i < mesh->number_of_vertices;i++)
            {
#ifdef _DEBUG_DELETING                                
                char s1[GM_VERTEX_BUFFER_SIZE];

                LOG("Clearing vertex #%i [%s]\n",i,vertexPath((GObject*)&mesh->mesh[i],s1,sizeof(s1)));
#endif                
                
                // Vertexes are refCounted and loose selection here!
                if (0 == (mesh->mesh[i].flags & GM_GOBJECT_FLAG_INVISIBLE))
                {
                    mesh->mesh[i].flags &= ~GM_FLAG_SELECTED;
                    mesh->mesh[i].flags |=  GM_GOBJECT_FLAG_INVISIBLE;
                    mesh->mesh[i].refCount--;
                }
            }
        }
        else
        {
            mesh->max_number_of_vertices = 0;
        }
        
        mesh->number_of_vertices = 0;
        mesh->real_number_of_vertices = 0;        
    }            
}

long meshSize(const CObject *obj)
{
    const Mesh *mesh = (const Mesh *)isCObject(OBJ_MESH,(const GObject*)obj);
    if (mesh)
    {
        return mesh->real_number_of_vertices;
    }            
    
    return 0;
}

GObject *iterateMesh(Iter *iter)
{
    Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,iter->object);
    if (mesh)
    {
        if ((NULL == iter->s1) || (NULL == iter->s2))
        {
            if (mesh->mesh)
            {
                iter->s1 = (GObject*)&mesh->mesh[0];
                iter->s2 = (GObject*)&mesh->mesh[mesh->number_of_vertices-1];
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
            iter->s1 = (GObject*)(((char *)iter->s1) + sizeof(mesh->mesh[0]));
                        
            return isObject(next);
        }
    }
    
    return NULL;
}


const GObject *citerateMesh(CIter *iter)
{
    const Mesh *mesh = (const Mesh*)isCObject(OBJ_MESH,iter->object);
    if (mesh)
    {
        if ((NULL == iter->s1) || (NULL == iter->s2))
        {
            if (mesh->mesh)
            {
                iter->s1 = (const GObject*)&mesh->mesh[0];
                iter->s2 = (const GObject*)&mesh->mesh[mesh->number_of_vertices-1];
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
            iter->s1 = (GObject*)(((const char *)iter->s1) + sizeof(mesh->mesh[0]));
                        
            return isObject(next);
        }
    }
    
    return NULL;
}

int meshExtend(Mesh *mesh, int size)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh) && (0 < size))
    {    
        char s1[GM_VERTEX_BUFFER_SIZE];
        
        unsigned int old_size = mesh->max_number_of_vertices;
        mesh->max_number_of_vertices = mesh->max_number_of_vertices + size;
        mesh->mesh = (Vertex*)memory_realloc(mesh->mesh, mesh->max_number_of_vertices * sizeof(Vertex));

        if (mesh->mesh)
        {
            memset(&mesh->mesh[old_size],0,sizeof(Vertex) * size);
            
            int count = 0;
            for (Vertex *v = &mesh->mesh[old_size]; 
                v <= &mesh->mesh[mesh->max_number_of_vertices-1];
                v++)
            {
                if (objectInit((GObject*)v, (GObject*)mesh,/* index */  0, OBJ_VERTEX))
                {     
                    // static memory not dynamic. Carefull when we delete
                    v->type |= _SOBJ_MARKER;
                    count++;
                }
            }   
            
            LOG("[%s] extended %i->%i by %i vertices\n",
                vertexPath((GObject*)mesh,s1,sizeof(s1)),
                old_size,mesh->max_number_of_vertices,count);
            
            return 0;
        }
        
        FATAL("Could mot extend [%s] by %i vertices to %i\n",
              vertexPath((GObject*)mesh,s1,sizeof(s1)),
              size,mesh->max_number_of_vertices);
    }    
    
    return -1;
}

Vertex* meshAddPoint(Mesh *mesh, EPoint *x)
{
    if (mesh && x)
    {      
        if (mesh->number_of_vertices >= mesh->max_number_of_vertices)
        {
            meshExtend(mesh,500);
        }

        Vertex *my_vertex = &mesh->mesh[mesh->number_of_vertices++];

        CopyEPoint(my_vertex->x,x);
        
        my_vertex->flags &= ~GM_GOBJECT_FLAG_INVISIBLE;
        
        if (0 < my_vertex->refCount)
        {
            char s1[GM_VERTEX_BUFFER_SIZE];
            ERROR("New vertex [%s] already has ref count of %i\n",
                vertexPath((GObject*)my_vertex,s1,sizeof(s1)),
                my_vertex->refCount);
        }
        my_vertex->refCount++;
        mesh->real_number_of_vertices++;
        
        vertexAddToBoundingBox((CObject*)mesh,my_vertex);
        
        return my_vertex;
    }
    
    return NULL;        
}


Vertex* meshAddVertex(Mesh *mesh, Vertex *v)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh) && isGObject(OBJ_VERTEX,v))
    {      
        if (0 == (GM_GOBJECT_FLAG_INVISIBLE & v->flags))
        {
            if (mesh->number_of_vertices >= mesh->max_number_of_vertices)
            {
                meshExtend(mesh,500);
            }

            Vertex *my_vertex = &mesh->mesh[mesh->number_of_vertices++];
            
            my_vertex->connections = v->connections;
            my_vertex->number_of_connections = v->number_of_connections;
            my_vertex->max_number_of_connections = v->max_number_of_connections;
            v->connections = 0;
            v->number_of_connections = 0;
            v->max_number_of_connections = 0;
            
            my_vertex->first_attrib = v->first_attrib;
            my_vertex->last_attrib = v->last_attrib;
            
            v->first_attrib = NULL;
            v->last_attrib = NULL;

            my_vertex->flags &= ~GM_GOBJECT_FLAG_INVISIBLE;
            
            if (0 < my_vertex->refCount)
            {
                char s1[GM_VERTEX_BUFFER_SIZE];
                ERROR("New vertex [%s] already has ref count of %i\n",
                    vertexPath((GObject*)my_vertex,s1,sizeof(s1)),
                    my_vertex->refCount);
            }
            my_vertex->refCount++;
            mesh->real_number_of_vertices++;
            
            vertexDestroy(v);
            
            vertexAddToBoundingBox((CObject*)mesh,my_vertex);
            
            return my_vertex;
        }
    }
    
    return NULL;    
}

int meshRemoveVertex(CObject *object, GObject *obj)
{
    char s1[GM_VERTEX_BUFFER_SIZE];
    char s2[GM_VERTEX_BUFFER_SIZE];
    
    Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
    Vertex *v = (Vertex*)isCObject(OBJ_VERTEX,obj);
    if (mesh && v)
    {
        for (int i = 0; i < (int)mesh->number_of_vertices; i++)
        {
            if (&mesh->mesh[i] == v)
            {
                v->flags |= GM_GOBJECT_FLAG_INVISIBLE;
                
                mesh->real_number_of_vertices--;
#ifdef _DEBUG                
                if (mesh->mesh[i].flags & GM_GOBJECT_FLAG_INVISIBLE)
                {
                    LOG("Removed [%s] from [%s]\n",
                         vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                         vertexPath((GObject*)object,s2,sizeof(s2)));                    
                }
#endif                
                return 0;
            }
        }
    }
    
    ERROR("Failed to remove [%s] from [%s]\n",
          vertexPath((GObject*)object,s1,sizeof(s1)),
          vertexPath((GObject*)object,s2,sizeof(s2)));
    
    return 1;
}


static int meshPaint(ViewerDriver *driver, struct GObjectT *object)
{
    Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
    if (mesh && driver && driver->drawDot)
    {
        if (0 == (mesh->flags & GM_GOBJECT_FLAG_INVISIBLE))
        {
            unsigned long triangulation_color = 0;
            
            unsigned long color = driver->getColorFor(GAPERAL_MESH);

            if (mesh->material)
            {
                color = mesh->material->color;
            }
            
            if (mesh->flags & GM_FLAG_SELECTED)
            {
                color = driver->getColorFor(GAPERAL_MESH_SELECTED);
            }
            
            if (GM_GOBJECT_FLAG_SHOW_TRI & mesh->flags)
            {
                triangulation_color = driver->getColorFor(GAPERAL_TRI);
            }
            
            for (unsigned  i = 0; i < mesh->number_of_vertices; i++)
            {
                Vertex *vertex = &mesh->mesh[i];
                if (0 == (vertex->flags & GM_GOBJECT_FLAG_INVISIBLE))
                {
                    if (vertex && (GM_GOBJECT_FLAG_SHOW_TRI & mesh->flags))
                    {    
                        if (0 < vertex->number_of_connections)
                        {
                            driver->setDrawingColor(triangulation_color);
                            for (int i = 0; i < vertex->number_of_connections; i++)
                            {
                                Vertex *vert2 = vertex->connections[i];

                                if (vert2)
                                {                        
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
                                }
                            }
                            // driver->setDrawingColor(color);
                        }
                    }
                
                    if (GM_GOBJECT_FLAG_SHOW_NORM & mesh->flags)
                    {
                        if (vertex->first_attrib  && (vertex->flags & GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID))
                        {
                            driver->setDrawingColor(driver->getColorFor(GAPERAL_NORMALE));
                            
                            const AreaAttributes *attributes = vertex->first_attrib;
                            while(attributes)
                            {
                                if (attributes->flags & GM_ATTRIB_FLAG_VALID)
                                {
                                    EPoint n[3];
                                    const Normale *normale = &attributes->normale;
                                    n[0] = normale->p[0] + NORM_LENGTH*normale->normal[0];
                                    n[1] = normale->p[1] + NORM_LENGTH*normale->normal[1];
                                    n[2] = normale->p[2] + NORM_LENGTH*normale->normal[2];
                                    driver->drawLine(normale->p, n);                    
                                }
                                attributes = (AreaAttributes*)attributes->next;                        
                            }
                        }
                    }

                    if (GM_GOBJECT_VERTEX_FLAG_MARK_ERROR & vertex->flags)
                    {
                        // This vertex gets a marker
                        driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARKERROR),30);
                    }
                    else
                    if (GM_GOBJECT_VERTEX_FLAG_MARK & vertex->flags)
                    {
                        // This vertex gets a marker
                        driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARK),25);
                    }
                    else
                    if (GM_GOBJECT_VERTEX_FLAG_MARK2 & vertex->flags)
                    {
                        // This vertex gets a marker
                        driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARK2),25);
                    }
                    
                    driver->setDrawingColor(color);
                    
                    driver->drawDot(vertex->x);
                }
            }            
        }
    }
    
    return 0;
}
