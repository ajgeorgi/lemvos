#include <string.h>
#include <stdio.h>
#include <math.h>

/* Solid for GM 20.11.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define __GOBJECT_PRIVATE
#include "gobject.h"
#undef __GOBJECT_PRIVATE

#include "vertex.h"
#include "modelstore.h"
#include "mea.h"


int paintSolid(ViewerDriver *driver, GObject *object);
static int solidRotateSolid(CObject *solid, double yaw, double roll, double pitch, double heave);
int solidRemoveVertex(CObject *object, GObject *v);

static Iter createIterate(CObject *object, flag_type mask);
static CIter createConstIterate(const CObject *object, flag_type mask);
static GObject *iterateSolid(Iter *iter);
static const GObject *citerateSolid(CIter *iter);
static void solidDestroy(CObject *object);

static long solidSize(const CObject *obj);


Solid *createSolid(GObject *parent)
{
    Solid *solid = memory_aloc(sizeof(Solid));
    memset(solid,0,sizeof(Solid));
    objectInit((GObject*)solid, parent, 0, OBJ_SOLID);    

    solid->methods.paint = paintSolid;
    solid->methods.iterate = iterateSolid;
    solid->methods.citerate = citerateSolid;
    solid->methods.rotate = solidRotateSolid;
    solid->methods.remove = solidRemoveVertex;
    solid->methods.creatIterator = createIterate;
    solid->methods.creatConstIterator = createConstIterate;
    solid->methods.destroy = solidDestroy;
    solid->methods.objectSize = solidSize;
    
    // objectInit((GObject*)&solid->original_box, (GObject*)solid, 0, OBJ_BOX);
    
    gobjectClearBoundingBox(&solid->original_box);
    solid->original_box.flags |= GO_BB_ORIGINAL_DATA;
    
    return solid;
}

long solidSize(const CObject *obj)
{
    if (obj)
    {
        long count = 0;
        CIter iter = gobjectCreateConstIterator(obj,0);
        
        for (const GObject *obj = gobjectConstIterate(iter); obj; obj = gobjectConstIterate(iter))
        {
            count ++;
        }    
        
        return count;
    }
    return 0;
}

static Iter createIterate(CObject *object, flag_type mask)
{
    Iter iter;
    memset(&iter,0,sizeof(Iter));
    
    iter.object = isCObject(ANY_COBJECT,object);
    iter.mask = mask & ~GM_GOBJECT_FLAG_POLY_ORIGINAL;
    
    // s1 supposed to be a Vertex*
    // s2 supposed to be a Polygon*
    return iter;
}

static CIter createConstIterate(const CObject *object, flag_type mask)
{
    CIter iter;
    memset(&iter,0,sizeof(CIter));
    
    iter.object = isCObject(ANY_COBJECT,object);
    iter.mask = mask;

    // s1 supposed to be a Vertex*
    // s2 supposed to be a Polygon*
    
    return iter;
}


GObject *iterateSolid(Iter *iter)
{
    if (iter)
    {
        Solid *solid = (Solid*)isCObject(OBJ_SOLID,iter->object);
        if (solid)
        {
            GObject *next = NULL;
            if (NULL == iter->s1)
            {                
                iter->s2  = (GObject *)solid->first;
                for (; 
                    iter->s2 && (iter->mask != (iter->s2->flags & iter->mask)); 
                    iter->s2 = (GObject*)iter->s2->next);
                
                    
                if (iter->s2)
                {   
                    Polygon* poly = (Polygon*)isGObject(OBJ_POLYGON,iter->s2);                        
                    if (poly && (iter->mask == (iter->s2->flags & iter->mask)))
                    {
                        iter->s1 = (GObject*)poly->first;
                    }
                }                    
                
                iter->s2 = NULL;
                
                return iter->s1; // First vertex of first matching polygon
            }
            else
            {
                next = iter->s1->next; // next vertex
                iter->s1 = next;
            
                if (next && (NULL == next->next))
                {   
                    // last vertex of current polygon ...                    
                    if (iter->s1->parent)
                    {
                        // ... need new polygon
                        iter->s2 = (GObject*)isGObject(OBJ_POLYGON,iter->s1->parent->next);
                        
                        for (; 
                            iter->s2 && (iter->mask != (iter->s2->flags & iter->mask)); 
                            iter->s2 = (GObject*)iter->s2->next);
                        
                        if (iter->s2)
                        {
                            if (iter->mask != (iter->s2->flags & iter->mask))
                            {
                                // No match
                                iter->s2 = NULL;
                            }
                        }
                    }
                }        
            }

            if (NULL == next)
            {
                if (iter->s2)
                {
                    Polygon* poly = (Polygon*)isGObject(OBJ_POLYGON,iter->s2);
                    if (poly)
                    {
                        iter->s1 = (GObject*)poly->first;
                        // char s1[GM_VERTEX_BUFFER_SIZE];
                        next = iter->s1;
                        // LOG("New polygon [%s]\n",vertexPath(next,s1,sizeof(s1)));
                    }
                    
                    iter->s2 = NULL;
                }
            }

#ifdef _DEBUG    
            if (next && (NULL == isGObject(OBJ_VERTEX,next)))
            {
                char s1[GM_VERTEX_BUFFER_SIZE];
                FATAL("Invalid vertex [%s]\n",vertexPath(next,s1,sizeof(s1)));
            }
#endif    
            
            return next;
        }
    }
    
    return NULL;
}

const GObject *citerateSolid(CIter *iter)
{
    if (iter)
    {
        const Solid *solid = (const Solid*)isCObject(OBJ_SOLID,iter->object);
        if (solid)
        {
            const GObject *next = NULL;
            if (NULL == iter->s1)
            {
                iter->s2  = (const GObject *)solid->first;
                if (iter->mask & GM_GOBJECT_FLAG_POLY_ORIGINAL)
                {
                    iter->s2 = (const GObject *)solid->first_original;
                }
                
                for (; 
                    iter->s2 && (iter->mask != (iter->s2->flags & iter->mask)); 
                    iter->s2 = (const GObject*)iter->s2->next);
                
                    
                if (iter->s2)
                {   
                    const Polygon* poly = (const Polygon*)isGObject(OBJ_POLYGON,iter->s2);                        
                    if (poly && (iter->mask == (iter->s2->flags & iter->mask)))
                    {
                        iter->s1 = (const GObject*)poly->first;
                    }
                }
                
                iter->s2 = NULL;

                return iter->s1;
            }
            else
            {
                next = iter->s1->next;
                iter->s1 = next;
            
                if (next && (NULL == next->next))
                {        
                    if (NULL == iter->s2)
                    {
                        iter->s2 = (const GObject*)isGObject(OBJ_POLYGON,iter->s1->parent->next);
                    }
                    
                    for (; 
                        iter->s2 && (iter->mask != (iter->s2->flags & iter->mask)); 
                        iter->s2 = (const GObject*)iter->s2->next);
                    
                    if (iter->s2)
                    {
                        if (iter->mask != (iter->s2->flags & iter->mask))
                        {
                            iter->s2 = NULL;
                        }
                    }
                }        
            }

            if (NULL == next)
            {
                if (iter->s2)
                {
                    const Polygon* poly = (Polygon*)isGObject(OBJ_POLYGON,iter->s2);
                    if (poly)
                    {
                        iter->s1 = (const GObject*)poly->first;
                        next = iter->s1;
                    }
                    
                    iter->s2 = NULL;
                }
            }

#ifdef _DEBUG    
            if (next && (NULL == isGObject(OBJ_VERTEX,next)))
            {
                char s1[GM_VERTEX_BUFFER_SIZE];
                FATAL("Invalid vertex [%s]\n",vertexPath(next,s1,sizeof(s1)));
            }
#endif    
            
            return next;
        }
    }
    
    return NULL;
}



int _plotterPlotPolygonAbsolut(ViewerDriver *driver, unsigned long long flags, const Polygon *poly, unsigned long color)
{    
    if (poly && (poly->flags & GM_GOBJECT_FLAG_POLY_ABSOLUT))
    {    
        unsigned long triangulation_color = 0;
        
        if (poly->flags & GM_FLAG_SELECTED)
        {
            LOG("Poly%i selected\n",poly->index);
            color = driver->getColorFor(GAPERAL_POLY_SELECTED);
        }
        
        if (GM_GOBJECT_FLAG_SHOW_TRI & flags)
        {
            triangulation_color = driver->getColorFor(GAPERAL_TRI);
        }
        
#ifdef _DEBUG_PLOT        
        int vertexCount = 0;
        int normalCount = 0;
#endif        
        const Vertex *last_point = NULL;
        for (const Vertex *vertex = (Vertex*)poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
        {          
            if (flags & GM_FLAG_MARK_START_END)
            {
                if (vertex == poly->first)
                {
                    // The first vertex gets a marker
                    driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_POLY_START),20);
                }

                if (NULL == vertex->next)
                {
                    //  The last vertex gets a marker
                    driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_POLY_END),20);
                }
            }
                
            if (last_point)
            {
                unsigned long vcolor = color;
                if ((vertex->flags & GM_GOBJECT_VERTEX_FLAG_INVALID) && (last_point->flags & GM_GOBJECT_VERTEX_FLAG_INVALID))
                {
                    vcolor = driver->getColorFor(GAPERAL_ERROR);
                }
                // Assign color for plot
                driver->setDrawingColor(vcolor);
                
                // Draw the plot absolut from p_(i-1) to p_i
#ifdef _DEBUG_PLOT                
                char lstr[50],vstr[50];
                LOG("%s->%s\n",
                        vertexToString(last_point,lstr,sizeof(lstr),GM_V2S_FLAGS_DEBUG),
                        vertexToString(vertex,vstr,sizeof(vstr),GM_V2S_FLAGS_DEBUG));
#endif
                
                driver->drawLine(last_point->x, vertex->x);
#ifdef _DEBUG_PLOT                                        
                vertexCount++;
#endif
                
                
            }


            if (GM_GOBJECT_FLAG_SHOW_TRI & flags)
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
                
            }
            
            if (GM_GOBJECT_FLAG_SHOW_NORM & flags)
            {
                if (vertex->first_attrib  && (vertex->flags & GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID))
                {
                    driver->setDrawingColor(driver->getColorFor(GAPERAL_NORMALE));
                    
                    const AreaAttributes *attributes = vertex->first_attrib;
#ifdef _DEBUG_PLOT                                            
                    int count = 0;
#endif                    
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
#ifdef _DEBUG_PLOT                        
                        normalCount++;
                        count++;
                        char p1str[50];
                        char p2str[50];
                        char p3str[50];
                        LOG("N%i, (P%i.V%i, P%i.V%i, P%i.V%i), p=%s\n",count,
                               attributes->v1->parent->index,attributes->v1->index,
                               vertexToString(attributes->v1,p1str,sizeof(p1str),GM_V2S_FLAGS_INDEX_ONLY),
                               vertexToString(attributes->v2,p2str,sizeof(p2str),GM_V2S_FLAGS_INDEX_ONLY),
                               vertexToString(attributes->v3,p3str,sizeof(p3str),GM_V2S_FLAGS_INDEX_ONLY),
                               EPoint3ToString(attributes->normale.p,pstr,sizeof(pstr))
                              );
#endif                        
                        
                        attributes = (AreaAttributes*)attributes->next;                        
                    }
                }
            }

            if (GM_GOBJECT_VERTEX_FLAG_MARK_ERROR & vertex->flags)
            {
                // This vertex gets a marker
                driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_MARKERROR),25);
            }
            else
            if (GM_FLAG_SELECTED & vertex->flags)
            {
                // This vertex gets a marker
                driver->drawPoint(vertex->x,driver->getColorFor(GAPERAL_VERTEX_SELECTED),15);
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
            
            last_point = vertex;                        
        }
        
#ifdef _DEBUG_PLOT                        
        LOG("Plotted:P%i: vertices= %i, normals=%i\n",poly->index,vertexCount,normalCount);
#endif
        return 0;
    }
    
    return -1;
}

int _plotterPlotPolygonRelative(ViewerDriver *driver, unsigned long long flags, const Polygon *poly, unsigned long color)
{    
    EPoint pi[GM_VERTEX_DIMENSION] = {0,0,0}; 
    EPoint pi_old[GM_VERTEX_DIMENSION];
    
    // fprintf(stderr,"first x = %s, p = %s\n",EPoint3ToString(pi_old,textbuffer,sizeof(textbuffer)),EPoint3ToString(pi_old,textbuffer2,sizeof(textbuffer2)));

    // Calculate the absolut starting point of the polygon
    pi_old[0] = poly->p[0];
    pi_old[1] = poly->p[1];
    pi_old[2] = poly->p[2];

    /*
    const EPoint *solid_p = solid->p;

    pi_old[0] += solid_p[0];
    pi_old[1] += solid_p[1];
    pi_old[2] += solid_p[2];
    */
    
    if (poly->flags & GM_FLAG_SELECTED)
    {
        LOG("Poly%i selected\n",poly->index);
        color = driver->getColorFor(GAPERAL_POLY_SELECTED);
    }
    
    int vertexCount = 0;
    const Vertex *last_vertex = NULL;
    for (const Vertex *vertex = poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
    {          
        // p_i = x + p_(o-1) (p absolut, x relativ)
        AddEPoints(pi,vertex->x,pi_old);

        if (poly->flags & GM_FLAG_MARK_START_END)
        {
            if (vertex == poly->first)
            {
                // The first vertex gets a marker
                driver->drawPoint(pi_old,driver->getColorFor(GAPERAL_POLY_START),10);
            }

            if (NULL == vertex->next)
            {
                //  The last vertex gets a marker
                driver->drawPoint(pi,driver->getColorFor(GAPERAL_POLY_END),10);
            }
        }

        if (GM_FLAG_SELECTED & vertex->flags)
        {
            // This vertex gets a marker
            driver->drawPoint(pi,driver->getColorFor(GAPERAL_VERTEX_SELECTED),15);
        }
        else
        if (GM_GOBJECT_VERTEX_FLAG_MARK & vertex->flags)
        {
            // This vertex gets a marker
            driver->drawPoint(pi,driver->getColorFor(GAPERAL_VERTEX_MARK),15);
        }

        // Assign color for plot
        driver->setDrawingColor(color);

        // Draw the plot absolut from p_(i-1) to p_i
        int err = driver->drawLine(pi_old, pi);

        if (0 > err)
        {
            driver->printError(0, "Draw failed\n");
            return -1;
        }

        if (last_vertex && (GM_GOBJECT_FLAG_SHOW_NORM & flags))
        {
            EPoint normale[3];
            vertexVectorProduct(normale, last_vertex->x, vertex->x);            
            double norm = vertexNorm(normale);
            
            if (fabs(norm) > 0.01)
            {
                EPointExtend(normale,(1/norm)*NORM_LENGTH,normale);
                AddEPoints(normale,normale,pi_old);
                
                driver->setDrawingColor(driver->getColorFor(GAPERAL_NORMALE));
                driver->drawLine(pi_old, normale);
            }
        }

        if (GM_GOBJECT_FLAG_SHOW_TRI & flags)
        {
            EPoint pxi[3];
            driver->setDrawingColor(driver->getColorFor(GAPERAL_TRI));
            
            for (int i = 0; i < vertex->number_of_connections; i++)
            {
                Vertex *v = vertex->connections[i];
                if (v)
                {
                    AddEPoints(pxi,pi,v->x);
                    driver->drawLine(pi, pxi);
                }
            }
        }

        
        // p_(i-1) = p_i
        pi_old[0] = pi[0];
        pi_old[1] = pi[1];
        pi_old[2] = pi[2];
        last_vertex = vertex;
        
        vertexCount++;
    }
    // fprintf(stderr,"Vertices plotted %i\n",vertexCount);
    
    return 0;
}

int paintSolid(ViewerDriver *driver, GObject *object)
{
    Solid *solid = (Solid*)object;
    
    unsigned long color = driver->getColorFor(GAPERAL_MODEL);

    if (solid->material)
    {
        color = solid->material->color;
    }
    
    if (solid->flags & GM_FLAG_SELECTED)
    {
        color = driver->getColorFor(GAPERAL_SOLID_SELECTED);
    }
    
    // fprintf(stderr,"%i : Plotting \"%s\" with material \"%s\"\n",plott_serial, solid->name,(solid->material ? solid->material->name : "None"));

    if (0 == (solid->flags & GM_GOBJECT_FLAG_INVISIBLE))
    {                               
        int apolyCount = 0;
        int rpolyCount = 0;
        
        for (const Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
        {    
            
            if (GM_GOBJECT_FLAG_POLY_ABSOLUT & poly->flags)
            {
                apolyCount++;
                _plotterPlotPolygonAbsolut(driver, solid->flags, poly, color);
            }
            else
            {
                rpolyCount++;
                _plotterPlotPolygonRelative(driver, solid->flags, poly, color);
            }
        }
#ifdef _DEBUG_PAINT                
        LOG("%s: abs=%i rel=%i polygones painted\n",solid->name,apolyCount,rpolyCount);
#endif        
        
    }
    
    return 0;
}

void solidDestroy(CObject *object)
{
    Solid *solid = (Solid *)isCObject(OBJ_SOLID,(GObject*)object);
    if (solid)
    {
        solid->refCount--;
        if (0 >= solid->refCount)
        {
            destroyAllMea((CObject*)solid);
            
            char s1[GM_VERTEX_BUFFER_SIZE];            
            
            for (Polygon *poly = solid->first; NULL != isObject((GObject*)poly); )
            {
                // Here should not be any original vertex or polygone
                if (poly->flags & GM_GOBJECT_FLAG_POLY_ORIGINAL)
                {
                    ERROR("Deleting original polygon [%s]",vertexPath((const GObject*)poly,s1,sizeof(s1)));
                }
                                
                for (Vertex *vertex = poly->first; NULL != isObject((GObject*)vertex); )
                {
                    Vertex *v = (Vertex*)vertex->next;
                    if (vertex->flags & GM_GOBJECT_FLAG_POLY_ORIGINAL)
                    {
                        ERROR("Deleting original vertex [%s]",vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE));
                    }
                    
                    vertexDestroy(vertex);
                    vertex = v;
                } 
                Polygon *p = (Polygon*)isObject(poly->next);
                memory_free(poly);
                poly = p;                
            }

            for (Polygon *poly = solid->first_original; NULL != isObject((GObject*)poly); )
            {
                for (Vertex *vertex = poly->first; NULL != isObject((GObject*)vertex); )
                {
                    Vertex *v = (Vertex*)vertex->next;
                    vertexDestroy(vertex);
                    vertex = v;
                } 
                Polygon *p = (Polygon*)isObject(poly->next);
                memory_free(poly);
                poly = p;                
            }
            
            if (solid->material)
            {
                vertexDestroyMaterial(solid->material);
                solid->material = NULL;
            }
            
            solid->magic = 0;

            if (solid->comments)
            {
                memory_free(solid->comments);
            }
            memory_free(solid);
        }
    }    
}


/*
static int solidTransformVertex(const CObject *object, const EPoint *v1, EPoint *v2)
{
    const long double *c = object->box.rotC;
    const long double *s = object->box.rotS;
    
    const long double rx1 = v1[0]*(c[1]*c[2] + s[0]*s[1]*s[2]) +  
                       v1[1]*(c[1]*-s[2] + s[0]*s[1]*c[2]) + 
                       v1[2]*(-c[0]*s[1]);
                       
    const long double ry1 = v1[0]*(c[0]*s[2]) +  
                       v1[1]*(c[0]*c[2]) + 
                       v1[2]*s[0];

    const long double rz1 = v1[0]*(s[1]*c[2] - s[0] * c[1]*s[2]) +
                       v1[1]*(- s[1]*s[2] - s[0] *c[1] * c[2] ) + 
                       v1[2]*(c[0] * c[1]);

    EPoint x1[3] = { rx1,ry1,rz1 };
    
    CopyEPoint(v2,x1);
    
    return 0;
}
*/

int solidRotateSolid(CObject *object, double yaw, double roll, double pitch, double heave)
{
    Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
    if (solid)
    {
        const EPointL cx = cosl(pitch);
        const EPointL sx = sinl(pitch);

        const EPointL cy = cosl(yaw);
        const EPointL sy = sinl(yaw);

        const EPointL cz = cosl(roll);
        const EPointL sz = sinl(roll);

        solid->box.rotC[0] = cx;
        solid->box.rotS[0] = sx;
        
        solid->box.rotC[1] = cy;
        solid->box.rotS[1] = sy;

        solid->box.rotC[2] = cz;
        solid->box.rotS[2] = sz;
        
        solid->box.yaw = yaw;
        solid->box.roll = roll;
        solid->box.pitch = pitch;

        // gobjectClearBoundingBox(&solid->box);
        // vertexMakeBBNormal(&solid->box);
        
        EPoint xheave[3] = { 0, heave,0 };
                 
        CopyEPoint(solid->p,xheave);
        CopyEPoint(solid->box.p,xheave);
        
        
        char s1[GM_VERTEX_BUFFER_SIZE];
        char s2[GM_VERTEX_BUFFER_SIZE];
        
        Polygon *trans_poly = NULL;
        for (const Polygon *original_poly = solid->first_original; isObject(original_poly); original_poly = (const Polygon *)original_poly->next)
        {
    #ifdef _DEBUG_TRANS        
            LOG("Transfoming polygon [%s]\n",vertexPath((const GObject*)original_poly,s1,sizeof(s1)));
    #endif        
            
            // Recycle polygon
            trans_poly = vertexSolidPolygonIterator(solid,trans_poly);
            
            if (NULL == trans_poly)
            {
                // Copy polygon
                trans_poly  = modelstoreCreatePolygon(solid);
                if (trans_poly)
                {
                    trans_poly->flags = (original_poly->flags & ~GM_GOBJECT_FLAG_POLY_ORIGINAL);
#ifdef _DEBUG_TRANS                            
                    LOG("New polygon [%s].flags=%s\n",
                        vertexPath((const GObject*)trans_poly,s1,sizeof(s1)),
                        vertexFlagsToString((const GObject*)trans_poly,s2,sizeof(s2))
                       );
#endif                    
                }
            }
            
            Vertex *v2 = NULL;
            for (const Vertex *v = original_poly->first; isObject(v); v = (const Vertex *)v->next)
            {
                // Recycle vertices            
                v2 = vertexPolygonIterator(trans_poly, v2);
                
                
                if (NULL == v2)
                {
                    // We need to copy the vertices
                    EPoint x1[3] = { 0,0,0 };
                    v2 = modelstoreAddToPolygon(trans_poly, x1);
                    // Copy any marker or settings
                    v2->flags |= v->flags & ~GM_GOBJECT_FLAG_POLY_ORIGINAL;
#ifdef _DEBUG_TRANS                            
                    LOG("New vertex [%s]\n",vertexToString(v2,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE));
#endif                                        
                }
                
                if (v2)
                {  
    #ifdef _DEBUG_TRANS                        
                    LOG("Transfoming vertex [%s]\n",vertexPath((const GObject*)v,s1,sizeof(s1)));
    #endif                
                    if (v2->flags & GM_GOBJECT_FLAG_POLY_ORIGINAL)
                    {
                        ERROR("Try to alter original vertex [%s]",vertexToString(v2,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE));
                    }
                    
                    if (vertexRotateVertex((CObject*)solid, v->x, v2->x))
                    {
                        ERROR("Rotation of [%s] -> [%s] failed.\n",vertexPath((const GObject*)v,s1,sizeof(s1)),
                        vertexPath((const GObject*)v2,s2,sizeof(s2)));
                    }

                    if (vertexAddToBoundingBox((CObject*)solid,v2))
                    {
                        ERROR("Failed to add [%s] to BB of \"%s\"",
                                vertexToString(v2,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG),
                                solid->name
                                );
                    }

                    AddEPoints(v2->x,v2->x,solid->p);
                }
            }
            
        }
        vertexRotateVertex((CObject*)solid, solid->normal.normal, solid->normal.normal);
        vertexRotateVertex((CObject*)solid, solid->normal.p, solid->normal.p);

        vertexRotateBB((CObject*)solid);
        
        return 0;
    }
    
    return -1;
}

int solidRemoveVertex(CObject *object, GObject *go)
{
    Solid *solid = (Solid *)isCObject(OBJ_SOLID,object);
    Vertex *v =  (Vertex *)isGObject(OBJ_VERTEX,go);
    if (solid && v)
    {
        Vertex *prev = NULL;
        Iter iter = gobjectCreateIterator(object,0);
        for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
        {
            Vertex *vertex = (Vertex *)isGObject(OBJ_VERTEX,obj);
            if (vertex)
            {
                if (v == vertex)
                {                
                    Polygon *poly = (Polygon *)isGObject(OBJ_POLYGON,vertex->parent);
                    if (poly)
                    {
                        if (prev)
                        {
                            prev->next = vertex->next;
                            
                            if (v == poly->last)
                            { 
                                poly->last = prev;
                            }
                        }
                        else{
                            poly->first = (Vertex*)vertex->next;
                        }
                        
#ifdef _DEBUG                
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        char s2[GM_VERTEX_BUFFER_SIZE];
                        LOG("Removed [%s] from [%s]\n",
                            vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                            vertexPath((GObject*)object,s2,sizeof(s2)));                    
#endif                
                        
                        vertexDestroy(v);
                        
                        return 0;
                    }
                    
                    return -1;
                }
                
                prev = v;
            }
        }        
    }
    
    return 1;
}