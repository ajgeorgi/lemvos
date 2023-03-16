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

Mesh *createMesh(GObject *parent)
{
    Mesh *mesh = memory_aloc(sizeof(Mesh));
    memset(mesh,0,sizeof(Mesh));
    objectInit((GObject*)mesh, parent, 0, OBJ_MESH);    

    mesh->methods.paint = meshPaint;
    mesh->methods.iterate = iterateMesh;
    mesh->methods.citerate = citerateMesh;    
    mesh->methods.remove = meshRemoveVertex;
    mesh->methods.creatIterator = createIterator;
    mesh->methods.creatConstIterator = createConstIterator;

    return mesh;
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

void meshDestroy(Mesh *mesh)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh))
    {        
        mesh->refCount--;
        if (0 >= mesh->refCount)
        {
            meshClear(mesh);
            destroyAllMea((CObject*)mesh);
            
            mesh->magic = 0;
            if (mesh->mesh)
            {
                memory_free(mesh->mesh);
            }
            if (mesh->material)
            {
                vertexDestroyMaterial(mesh->material);
            }
            
            if (mesh->comments)
            {
                memory_free(mesh->comments);
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
                // Vertexes are refCounted and loose selection here!
                mesh->mesh[i]->flags &= ~GM_FLAG_SELECTED;
                vertexDestroy(mesh->mesh[i]);
                mesh->mesh[i] = NULL;
            }
            
            mesh->number_of_vertices = 0;
        }
    }            
}

long meshSize(Mesh *mesh)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh))
    {
        return mesh->number_of_vertices;
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
            GObject *next = *((GObject **)iter->s1);
            iter->s1 = (GObject*)(((char *)iter->s1) + sizeof(mesh->mesh[0]));
                        
            return next;
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
            const GObject *next = *((const GObject **)iter->s1);
            iter->s1 = (GObject*)(((const char *)iter->s1) + sizeof(mesh->mesh[0]));
                        
            return next;
        }
    }
    
    return NULL;
}


int meshAddVertex(Mesh *mesh, Vertex *v)
{
    if (isCObject(OBJ_MESH,(GObject*)mesh) && isGObject(OBJ_VERTEX,v))
    {        
        if (mesh->number_of_vertices >= mesh->max_number_of_vertices)
        {
            unsigned int old_size = mesh->max_number_of_vertices;
            mesh->max_number_of_vertices = mesh->max_number_of_vertices+100;
            mesh->mesh = (Vertex**)memory_realloc(mesh->mesh,mesh->max_number_of_vertices * sizeof(Vertex*));
            
            if (old_size > 0)
            {
                memset(&mesh->mesh[old_size],0,(mesh->max_number_of_vertices-old_size)* sizeof(Vertex*));
            }
            else
            {
                memset(mesh->mesh,0,mesh->max_number_of_vertices* sizeof(Vertex*));
            }
        }
        
        mesh->mesh[mesh->number_of_vertices++] = v;
        v->refCount++;
        
        return vertexAddToBoundingBox((CObject*)mesh,v);
    }
    
    return -1;    
}

int meshRemoveVertex(CObject *object, GObject *obj)
{
    Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
    Vertex *v = (Vertex*)isCObject(OBJ_VERTEX,obj);
    if (mesh && v)
    {
        for (int i = 0; i < (int)mesh->number_of_vertices; i++)
        {
            if (mesh->mesh[i] == v)
            {
                memmove(&mesh->mesh[i],&mesh->mesh[i+1],(mesh->number_of_vertices - i)*sizeof(Vertex*));
                mesh->number_of_vertices--;
#ifdef _DEBUG                
                if (mesh->mesh[i] != v)
                {
                    char s1[GM_VERTEX_BUFFER_SIZE];
                    char s2[GM_VERTEX_BUFFER_SIZE];
                    LOG("Removed [%s] from [%s]\n",
                         vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                         vertexPath((GObject*)object,s2,sizeof(s2)));                    
                }
#endif                
                    
                vertexDestroy(v);
                return 0;
            }
        }
    }
    
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
                Vertex *vertex = (Vertex *)isGObject(OBJ_VERTEX,mesh->mesh[i]);
                
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
    
    return 0;
}
