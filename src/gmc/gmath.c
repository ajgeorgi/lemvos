#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GMath for GMC 10.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "modelstore.h"
#include "vertex.h"
#include "gmath.h"
#include "mea.h"
#include "sequencer.h"

#define GMATH_MEA_VALUE_NAME "# of attribs"

Vertex **_gmath_vertex_list = NULL;
int _number_of_vertices = 0;
int _max_number_of_vertices = 0;

CObject *gmathCalculateGravity(const char* name, CObject *object, unsigned long color);
CObject *gmathCalculateBouyancy(const char* name, CObject *object, unsigned long color);
CObject *gmathCalculateAttributes(const char* name, CObject *object, unsigned long color);


int gmathAreaNormal(Vertex *v2, EPoint *normal, EPoint *p,  Vertex *v1,  Vertex *v3);

int gmathInit()
{
    int err = sequencerAddCalculation(GMATH_MEA_GRAVITY_NAME, gmathCalculateGravity, GAPERAL_GRAVITY_MEA);
    err |= sequencerAddCalculation(GMATH_MEA_BOUYANCY_NAME, gmathCalculateBouyancy, GAPERAL_BOUYANCY_MEA);
    err |= sequencerAddCalculation(GMATH_MEA_ATTRIBUTES_NAME, gmathCalculateAttributes, GAPERAL_MEASUREMENT);

    _number_of_vertices = 0;
    
    return err;
}

int gmathAddVertex(Vertex *v)
{
    if (_number_of_vertices <= _max_number_of_vertices)
    {
        _max_number_of_vertices += 200;
        _gmath_vertex_list = memory_realloc(_gmath_vertex_list,_max_number_of_vertices * sizeof(Vertex*));
    }
    
    _gmath_vertex_list[_number_of_vertices] = v;
    _number_of_vertices++;
    
    return 0;
}

static int gmathRemoveDublicates(CObject *object)
{
    for (int i = 0; i < _number_of_vertices; i++)
    {            
        Vertex *v = _gmath_vertex_list[i];
        
        if (v)
        {
            for (int j = 0; j < _number_of_vertices; j++)
            {        
                if ((v == _gmath_vertex_list[j]) && (i != j))
                {
                    _gmath_vertex_list[j] = NULL;
                }        
                
                if (_gmath_vertex_list[j] && (i != j))
                {                
                    const double dist = vertexSphereDistance(v->x,_gmath_vertex_list[j]->x);
                    
                    if (dist < 0.00001)
                    {
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        char s2[GM_VERTEX_BUFFER_SIZE];
                        
                        ERROR("Dublicate vertex found:\n [%s],\n [%s]\n",
                                vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                                vertexToString(_gmath_vertex_list[j],s2,sizeof(s2),GM_V2S_FLAGS_DEBUG_VERBOSE));
                              
                       if (0 == vertexMergeVertices(object, v, _gmath_vertex_list[j]))
                       {
                           _gmath_vertex_list[j] = NULL;
                       }
                    }                        
                }
                
            }
        }
    }
    
    return 0;
}


Vertex *gmathFindClosest(const Vertex *v, int *index, double *distance)
{
    Vertex *best_match = NULL;
    double best_dist = HUGE_VAL;
    int best_index = -1;
    for (int j = 0; j < _number_of_vertices; j++)
    {
        if (_gmath_vertex_list[j] && (v != _gmath_vertex_list[j]))
        {                
            const double dist = vertexSphereDistance(v->x,_gmath_vertex_list[j]->x);
            
            if (dist < 0.00001)
            {
                char s1[GM_VERTEX_BUFFER_SIZE];
                char s2[GM_VERTEX_BUFFER_SIZE];
                
                ERROR("Dublicate vertex found:\n [%s],\n [%s]\n",
                        vertexToString(best_match,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(_gmath_vertex_list[j],s2,sizeof(s2),GM_V2S_FLAGS_DEBUG_VERBOSE));
            }            
            
            if (dist < best_dist)
            {
                best_match = _gmath_vertex_list[j];
                best_dist = dist;
                best_index = j;
            }            
        }
    }
    
    if (index)
    {
        *index = best_index;
    }
    
    if (distance)
    {
        *distance = best_dist;
    }
    
    return best_match;
}



int gmathClearVertexList()
{
    _number_of_vertices = 0;
    
    return 0;
}

int gmathPolygonLength(Polygon *poly)
{
    int vertexCount = 0;
    for (Vertex *v = poly->first; NULL != v; v = (Vertex*)v->next)
    {
        vertexCount++;
    }
    
    return vertexCount;
}

int interpolationFunction(EPoint *result, EPoint *p, Vertex *xim1, EPoint *x)
{
    result[0] = 1.0;
    result[1] = 1.0;
    result[2] = 1.0;
        
    int i = 0;
    for (Vertex *v = xim1; (NULL != v) && (i < 3) ; v = (Vertex*)v->next, i++)
    {        
        EPoint x_aps[3];
        
        AddEPoints(x_aps,p,v->x); 
        
        result[0] = result[0] * (x[0]-x_aps[0]);    
        result[1] = result[0] * (x[1]-x_aps[1]);
        result[2] = result[0] * (x[2]-x_aps[2]);
    }
    
    return 0;
}

Polygon *gmathPolynomInterpolation(Polygon *poly, Polygon *result, int resolution)
{
    EPoint xs[3];
    CopyEPoint(xs,poly->p);

    const double step_width = 1.0/((double)resolution);
    
    // Iterate x_(i-1)
    int i = 0;
    for (Vertex *xim1 = poly->first; NULL != xim1; xim1 = (Vertex*)xim1->next)
    {
        // step_width = |x_(i-1)|/resolution
        //const double norm = vertexNorm(xim1->x);
        
        fprintf(stderr,"SW_%i = %.3f, x_(i-1) = (%.3f, %.3f, %.3f)\n",i,step_width,xim1->x[0],xim1->x[1],xim1->x[2]);
        
        // d_j in [0,1]
        for (int j = 0; j <= resolution; j++)
        {
            EPoint x[3];
            const double d = step_width * j;
            
            // x = xs + d*x_(i-1)
            EPointExtend(x,d,xim1->x);
            AddEPoints(x,xs,x);
            fprintf(stderr,"xs = (%.3f, %.3f, %.3f)\n",xs[0],xs[1],xs[2]);
            
            // sx = s(x) = (x - x_(i-1))*(x - x_i)*(x - x_(i+1))
            EPoint sx[3]; // result of the polynom at x
            interpolationFunction(sx,xs,xim1,x);
            
            fprintf(stderr,"s(%.3f, %.3f, %.3f) = (%.3f, %.3f, %.3f) for d = %.3f\n",x[0],x[1],x[2],sx[0],sx[1],sx[2],d);
            
            modelstoreAddToPolygon(result, sx);
        }
        
        // xs = xs + x_(i-1)
        AddEPoints(xs,xs,xim1->x);
        i++;
    }
    
    return result;
}

int gmathPointNormalOut(const CObject *object, Vertex *v2)
{
    char s1[GM_VERTEX_BUFFER_SIZE];

    if (v2)
    {
        if (isCObject(ANY_COBJECT,object) && !vertexIsNull(object->box.center,0.0001))
        {
            AreaAttributes *attributes = v2->first_attrib;
            while(attributes)
            {            
                Normale *normale = &attributes->normale;
                EPoint *n1 = normale->p, n2[3];
                
                AddEPoints(n2,normale->p,normale->normal);
            
                EPoint dist1[3], dist2[3];
                
                DiffEPoints(dist1,object->box.center,n1);
                DiffEPoints(dist2,object->box.center,n2);

                if (vertexNorm(dist1) > vertexNorm(dist2))
                {
                    EPointExtend(normale->normal,-1,normale->normal);
                }
                
                // Normale is now pointing from surface out of the object
                attributes->flags &= ~GM_ATTRIB_FLAG_NORMALE_TO_FACE;
                attributes->flags &= ~GM_ATTRIB_FLAG_NORMALE_INSIDE;
                
                attributes = (AreaAttributes*)isGObject(OBJ_ATTRIB,attributes->next);
            }
            
            return 0;
        }

        if (NULL == v2->first_attrib)
        {
            ERROR("No normale found for [%s]\n",
                vertexPath((GObject*)v2,s1,sizeof(s1)));           
        }       
        else
        if (vertexIsNull(object->box.center,0.0001))
        {
            char s2[GM_VERTEX_BUFFER_SIZE];
            char s3[GM_VERTEX_BUFFER_SIZE];
            ERROR("Failed to flip normale of [%s], center of [%s]=%s\n",
                vertexPath((GObject*)v2,s1,sizeof(s1)),
                vertexPath((GObject*)object,s3,sizeof(s3)),
                EPoint3ToString(object->box.center,s2,sizeof(s2))
                );
       }
       
    }
    
    if (NULL == isCObject(ANY_COBJECT,object))
    {
        ERROR("Unknown object [%s]\n",
            vertexPath((GObject*)object,s1,sizeof(s1)));                   
    }
    
    
    return -1;
}

// v1,v2,v3 must be absolute vertices of the model
long double gmathTriangleArea(Vertex *v1, Vertex *v2, Vertex *v3, flag_type mask)
{
    EPoint  g2[3];
    int connect = (OBJ_POLYGON == objectGetType(v2->parent));
    
    if (connect)
    {
        if (mask != (v2->parent->flags & mask))
        {
            // Don't use these vertices for triangulation
            return 0;
        }   
    }
    
    vertexAddConnection(v2, v1);
    vertexAddConnection(v2, v3);        
        
    // Make the triangle relative
    DiffEPoints(g2,v3->x,v1->x);
                
    // Get the triangle area
    
    const double l2 = vertexNorm(g2);
    
    double distance = 0;
    if (fabs(l2) > 0.001)
    {        
        // No distance between v1 and v2. Calculate here perpendualr distance
        EPoint a[3],h[3],d[3];
        DiffEPoints(a,v2->x,v1->x);
        const double sa = vertexScalarProduct(a,g2);
        const double sg2 = vertexScalarProduct(g2,g2);
        EPointExtend(h,sa/sg2,g2);
        AddEPoints(h,h,v1->x);
        
        DiffEPoints(d,v2->x,h);
        distance = vertexNorm(d);        
    }
    

    EPoint g1[3],g3[3];
    DiffEPoints(g1,v1->x,v2->x);
    DiffEPoints(g3,v3->x,v2->x);
    
    EPoint nv[3];
    vertexVectorProduct(nv, g1, g3);
    
    AreaAttributes* attrib = NULL;
    EPoint norm[3],p[3];
    if (0 == gmathAreaNormal(v2, norm, p, v1, v3))
    {
#ifdef _DEBUG_NORM        
        char nstr[40],pstr[40];
        fprintf(stderr," V%i:norm=%s,p=%s, ",
                v2->index,
                EPoint3ToString(norm,nstr,sizeof(nstr)),
                EPoint3ToString(p,pstr,sizeof(pstr)));
#endif  
        
        attrib = vertexCreateAttribute(v2);
        attrib->refCount++;
        
        v2->flags |= GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;

        // Store normal to attributes
        CopyEPoint(attrib->normale.normal,norm);
        CopyEPoint(attrib->normale.p,p);

        attrib->flags |= GM_ATTRIB_FLAG_NORMALE_VALID;
        
        // Two different methods for the area
        attrib->area2 = vertexNorm(nv)/2.0;        
        attrib->area1 = (l2*distance)/2.0;
        
        // The triangle the attributes belong too
        attrib->v1 = v1;
        attrib->v2 = v2;
        attrib->v3 = v3;                
    }
    else
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        ERROR("Failed to calculate area normale at [%s]\n",
              vertexToString(v2,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE));
        v2->flags &= ~GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;       
        v2->flags |= GM_GOBJECT_VERTEX_FLAG_MARK_ERROR;
        return 0;
    }
    
#ifdef _DEBUG_TRIANG                       
    fprintf(stderr,"SA: A(P%i.V%i,P%i.V%i,P%i.V%i)=%.3f (%.3f) (P%i.V%i.connections=%i) \n",
            v1->parent->index,v1->index,
            v2->parent->index,v2->index,
            v3->parent->index,v3->index,
            attrib?attrib->area1:-1,attrib?attrib->area2:-1,
            v2->parent->index,v2->index,v2->number_of_connections);
#endif
    
    return attrib?attrib->area1:0;
}

// ???: v1,v2,v3 must be absolute vertices of the model
long double gmathTriangleArea2(Vertex *v1, Vertex *v2, Vertex *v3)
{
    EPoint g1[3],g3[3];
    
    vertexAddConnection(v2, v1);
    vertexAddConnection(v2, v3);

    DiffEPoints(g1,v1->x,v2->x);
    DiffEPoints(g3,v3->x,v2->x);
    
    EPoint nv[3];
    vertexVectorProduct(nv,g1,g3);
    
    return vertexNorm(nv)/2.0;
}

long double gmathSectionArea(Polygon *poly1, Polygon *poly2, flag_type mask)
{
    Vertex *v1 = vertexPolygonIterator(poly1, NULL);
    Vertex *v2 = vertexPolygonIterator(poly2, NULL);
    Vertex *v3 = vertexPolygonIterator(poly1, v1);
    
    long double sum_area = 0;
    
    //
    // Eventualy ignore empty triangles here to pass the algorithm with better results???
    //
    while (v1 && v2 && v3)
    {        
        sum_area += gmathTriangleArea(v1,v2,v3, mask);        
                
        Vertex *v4 = vertexPolygonIterator(poly2, v2);

        v1 = v3;
        
        if (v4)
        {
            sum_area += gmathTriangleArea(v2,v3,v4, mask);        
        
            /* v5 */ v3 = vertexPolygonIterator(poly1, v3);
        }
        
        v2 = v4;
    }
    
    return sum_area;
}

double gmathPolyDist(Polygon *poly1, Polygon *poly2)
{
    (void)poly1;
    (void)poly2;
    
    
    double min_dist = HUGE_VAL;
    Vertex *v1 = vertexPolygonIterator(poly1, NULL);
    Vertex *v2 = vertexPolygonIterator(poly2, NULL);
    Vertex *lv1 = NULL;
    Vertex *lv2 = NULL;
    Vertex *llv1 = NULL;
    Vertex *llv2 = NULL;
    Vertex *lllv1 = NULL;
    Vertex *lllv2 = NULL;

    while (v1 && v2)
    {
        EPoint d[3];
        
        DiffEPoints(d,v2->x,v1->x);
        
        double dist = vertexNorm(d);
        
        if (min_dist > dist)
        {
            min_dist = dist;
        }

        if (lv2)
        {
            DiffEPoints(d,lv2->x,v1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }

        if (lv1)
        {
            DiffEPoints(d,v2->x,lv1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }

        if (llv2)
        {
            DiffEPoints(d,llv2->x,v1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }

        if (llv1)
        {
            DiffEPoints(d,v2->x,llv1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }

        if (lllv2)
        {
            DiffEPoints(d,lllv2->x,v1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }

        if (lllv1)
        {
            DiffEPoints(d,v2->x,lllv1->x);
            
            dist = vertexNorm(d);
            
            if (min_dist > dist)
            {
                min_dist = dist;
            }
        }
        
        lllv1 = llv1;
        lllv2 = llv2;
        llv1 = lv1;
        llv2 = lv2;
        lv1 = v1;
        lv2 = v2;
        v1 = vertexPolygonIterator(poly1, v1);
        v2 = vertexPolygonIterator(poly2, v2);
    }
    
    return min_dist;
}

int gmathTriangFinalize(CObject *object, flag_type mask)
{    
    char s1[160];
    char s2[160];        
    char s3[160];
    char s4[160];
    LOG("Finalizing [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
    
    int number_of_attributes = 0;
    int failed_normale_proc = 0;
    int number_of_vertices = 0;
    int number_of_missing_attributes = 0;
    int number_of_invalid_attributes = 0;
    int number_of_duplicates = 0;
    int number_of_unconnected = 0;
    
    vertexMakeBBNormal(&object->box);
    gmathClearVertexList();
    
    Iter iter = gobjectCreateIterator(object,mask);
    for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
    {
        Vertex *v = (Vertex*)isGObject(OBJ_VERTEX,obj);
        if (v)
        {
    #ifdef _DEBUG_ITER        
            LOG("Vertex: [%s]\n",vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE));
    #endif
            number_of_vertices++;         
            if (gmathPointNormalOut(object, v))
            {
                failed_normale_proc++;
            }
            
            if (0 == v->number_of_connections)
            {
                // Unconnected vertices?
                v->flags |= GM_GOBJECT_VERTEX_FLAG_MARK_ERROR;
                ERROR("Unconnected vertex [%s]\n",vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE));
                number_of_unconnected++;
                gmathAddVertex(v);
            }  
            
            if (NULL == v->first_attrib)
            {
                // No attributes
                v->flags |= GM_GOBJECT_VERTEX_FLAG_MARK;
                LOG("No attributes at vertex [%s]\n",vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE));            
                number_of_missing_attributes++;
            }  
                            
            if (v->first_attrib)
            {
                AreaAttributes *attributes = v->first_attrib;
                
                while(attributes)
                {
                    if (attributes->flags & GM_ATTRIB_FLAG_VALID)
                    {
                        if (attributes->v1 && attributes->v2 && attributes->v3)
                        {                    
                            number_of_attributes++;
                        }
                    }
                    else
                    {
                        number_of_invalid_attributes++;
                    }
                    
                    attributes = (AreaAttributes *)attributes->next;
                }
            }
        }
    }
   
   gmathRemoveDublicates(object);

    /*
    double width, length, height;
    vertexGetCObjectSize(object, &width, &length, &height);
    
    const double eps = MAX(width,MAX(length,height)) / 100.0;
    */
    int number_of_real_dublicates = 0;
    for (int i = 0; i < _number_of_vertices; i++)
    {
        if (_gmath_vertex_list[i])
        {
            double best_dist = HUGE_VAL;
            int best_index = -1;            
            Vertex *v = _gmath_vertex_list[i];
            Vertex *best_match = gmathFindClosest(v, &best_index, &best_dist);
            
            if (0.0 == best_dist)
            {
                number_of_real_dublicates++;
            }
            
            if (best_match)
            {
                Vertex *parent1 = vertexConnectedParent(object, v, 0);
                Vertex *parent2 = vertexConnectedParent(object, best_match, 0);

                
                if (parent1 == parent2)
                {
                    LOG("Connecting: [%s] <-> [%s]\n",vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(best_match,s3,sizeof(s3),GM_V2S_FLAGS_DEBUG_VERBOSE));
                    
                    // Connect if they share a parent
                    vertexAddConnection(v, best_match);                    
                    
                    best_match->flags &= ~GM_GOBJECT_VERTEX_FLAG_MASK_MARKER;
                }
                else
                if (vertexIsConnected(parent1, parent2))
                {
                    LOG("Merging: [%s] <-> [%s]\n",vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(best_match,s3,sizeof(s3),GM_V2S_FLAGS_DEBUG_VERBOSE));
                    
                    // Do a merge if they don't share a parent but parents are connected
                    vertexMergeVertices(object, best_match, v);
                    
                    best_match->flags &= ~GM_GOBJECT_VERTEX_FLAG_MASK_MARKER;
                    best_match->flags |= GM_GOBJECT_VERTEX_FLAG_MARK;                    
                }
                else
                {
                    LOG("Untouched [%s]\n  <- [%s],\n  [%s]\n  -> [%s]\n",
                        vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(parent1,s2,sizeof(s2),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(parent2,s4,sizeof(s4),GM_V2S_FLAGS_DEBUG_VERBOSE),
                        vertexToString(best_match,s3,sizeof(s3),GM_V2S_FLAGS_DEBUG_VERBOSE)
                        );    
                    best_match->flags &= ~GM_GOBJECT_VERTEX_FLAG_MASK_MARKER;
                    best_match->flags |= GM_GOBJECT_VERTEX_FLAG_MARK2;                            
                }                
            }
        }
    }


    double min_count = 0;
    double max_count = 0;
    meaGetScalar(object, GMATH_MEA_ATTRIBUTES_NAME, GMATH_MEA_VALUE_NAME, &min_count, &max_count, NULL);
    
    Mea *solidData = meaGetMea(object, GMATH_MEA_ATTRIBUTES_NAME);
    Value *num_attribs = meaAddScalar(solidData, GMATH_MEA_VALUE_NAME, max_count, number_of_attributes, MeaUnit_Count);
    snprintf(num_attribs->description,sizeof(num_attribs->description),"Number of triangle attributes of \"%s\".",object->name);

    LOG("Finalized [%s]:# of vertices = %i,\n"
        "   # of failed normale processes = %i,\n"
        "   # of attributes = %i,\n"
        "   # of vertices with missing attributes = %i\n"
        "   # of invalid attributes = %i\n"
        "   # of duplicates = %i\n"
        "   # of unconnected = %i\n"
        "   # of real duplicates = %i\n",
        vertexPath((GObject*)object,s1,sizeof(s1)),
        number_of_vertices,
        failed_normale_proc,
        number_of_attributes,
        number_of_missing_attributes,
        number_of_invalid_attributes,
        number_of_duplicates,
        number_of_unconnected,
        number_of_real_dublicates
       );
    
    LOG_FLUSH;
    
#ifdef _DEBUG_MEMORY
    memory_check();
#endif    
        
    return 0;    
}


double gmathSolidArea(Solid *solid, flag_type mask)
{
    long double sum_area = 0;

    Polygon *last = NULL;
    int frame = 0;
    
    gmathRemoveDoubles(solid);
    gmathRemoveAttributes((CObject*)solid);

    for (Polygon *poly = vertexSolidPolygonIterator(solid, NULL); 
         NULL != poly; 
         poly = vertexSolidPolygonIterator(solid, poly))
    {
        if (poly)
        {
            vertexRemoveConnectionsFromPoly(poly);

            if ((mask |GM_GOBJECT_FLAG_POLY_ABSOLUT) == (poly->flags & (mask | GM_GOBJECT_FLAG_POLY_ABSOLUT)))
            {
                frame++;
                
                if (last)
                {
                    const long double area = gmathSectionArea(last,poly,mask);
#ifdef _DEBUG_TRIANG                                        
                    LOG("Area of P%i<->P%i = %'.3Lf\n",last->index,poly->index,area);
#endif                    
                    sum_area += area;
                }
                
                last = poly;
            }
        }
    }
    
#ifdef _DEBUG    
    LOG("Number of frames processed: %i, Area = %.3Lf\n",frame,sum_area);
#endif    

    Mea *solidData = meaGetMea((CObject*)solid, GMATH_MEA_ATTRIBUTES_NAME);

    Value *num_frames = meaAddScalar(solidData, "# of Frames", frame, frame, MeaUnit_Count);
    snprintf(num_frames->description,sizeof(num_frames->description),"Number of frames of \"%s\".",solid->name);
    
    return sum_area;
}

int gmathObjectVolume(CObject* object, EPointL *max_volume, EPointL *min_volume, EPointL *max_area, EPointL *min_area)
{
    if (isCObject(ANY_COBJECT,object))
    {
        int err = 0;
        
        EPointL area_max = 0;
        EPointL area_min = 0;

        EPointL volume_max = 0;
        EPointL volume_min = 0;
        
        int number_of_attributes = 0;
        int number_of_failed_attributes = 0;
        int number_of_vertices = 0;
        
        // ??? Celling of the bounding box is sufficient for a convex object???
        EPoint x4[3];
        AddEPoints(x4,object->box.top_center,object->box.p);
        
        char s1[GM_VERTEX_BUFFER_SIZE];

        Iter iter = gobjectCreateIterator(object,0);
        for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
        {
            Vertex *vertex = (Vertex*)isGObject(OBJ_VERTEX,obj);
            if (vertex)
            {
                AreaAttributes *attrib = vertex->first_attrib;
                while(attrib)
                {
                    if (attrib->flags & GM_ATTRIB_FLAG_VALID)
                    {
                        area_max += MAX(attrib->area1,attrib->area2);
                        area_min += MIN(attrib->area1,attrib->area2);
                        
                        if (attrib->v1 && attrib->v2 && attrib->v3)
                        {
                            EPoint g1[3], g2[3], g3[3];
                            
                            DiffEPoints(g1,attrib->v1->x,attrib->v2->x);
                            DiffEPoints(g2,attrib->v3->x,attrib->v2->x);
                            DiffEPoints(g3,x4,attrib->v2->x);
                            
                            attrib->volume = fabsl(vertexDet3(g1,g2,g3)) / 6.0;
                            
                            volume_max += attrib->volume ;
                            volume_min += attrib->volume ;
                            
                            vertex->flags |= GM_GOBJECT_VERTEX_FLAG_SEEN;
                            // attrib->v3 = x4;
                            
                            number_of_attributes++;
                        }
                        else
                        {
                            ERROR("Volume: missing triangle at [%s]\n",vertexPath((GObject*)attrib,s1,sizeof(s1)));
                            err = 1;
                        }
                    }
                    else
                    {
                        number_of_failed_attributes++;
                    }
                    attrib = (AreaAttributes*)attrib->next;
                }
                
                number_of_vertices++;
            }
        } 

        if (max_volume)
        {
            *max_volume = volume_max;
        }
        if (min_volume)
        {
            *min_volume = volume_min;
        }
        if (max_area)
        {
            *max_area = area_max;
        }
        if (min_area)
        {
            *min_area = area_min;
        }
        
        Mea *area_mea = meaGetMea(object, GMATH_MEA_ATTRIBUTES_NAME);
        if (area_mea)
        {
            double min_count = 0;
            double max_count = 0;
            meaGetScalar(object, GMATH_MEA_ATTRIBUTES_NAME, GMATH_MEA_VALUE_NAME, &max_count, &min_count, NULL);

            Value *num_attribs = meaAddScalar(area_mea, GMATH_MEA_VALUE_NAME, number_of_attributes, min_count, MeaUnit_Count);
            snprintf(num_attribs->description,sizeof(num_attribs->description),"Number of triangle attributes of \"%s\".",object->name);
        }
        else
        {
            ERROR("Can not create data entry \"%s\" for [%s]\n",
                  GMATH_MEA_ATTRIBUTES_NAME,
                  vertexPath((GObject*)object,s1,sizeof(s1)));
            err = 1;
        }
        
        return err;
    }
    
    return -1;
}


int _gmathAddCrossingVertexToMesh(Mesh *mesh, const Vertex *out, Vertex *in, const EPoint* normal, const EPoint *p)
{
    const long double distP = vertexScalarProduct(normal,p);
    EPoint pinch[3];
    EPoint line[3];
            
    Vertex *inside = NULL;
    int err = 0;
     for (int i = 0; (0 == err) && (i < in->number_of_connections); i++)
     {
        Vertex *connected = in->connections[i];
         
        const long double distV = vertexScalarProduct(connected->x,normal);         
        
        if (distV-distP > 0)
        {
            // Out ...
            // Cut to inside
             
            connected->flags &= ~GM_GOBJECT_VERTEX_FLAG_MCONNECTED;
                        
            if (NULL == inside)
            {
                // Need new inside vertex for new connections
                inside = meshAddPoint(mesh,in->x);
                inside->flags = (in->flags & ~GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID);
                
                AreaAttributes *attrib = in->first_attrib;
                while(attrib)
                {
                    if (attrib->flags & GM_ATTRIB_FLAG_VALID)
                    {
                        Vertex *v1 = attrib->v1;
                        Vertex *v3 = attrib->v3;
                        
                        // Is triangle complete inside?
                        const long double distV = vertexScalarProduct(v1->x,normal);         
                        if (distV-distP <= 0)
                        {
                            const long double distV = vertexScalarProduct(v3->x,normal);         
                            if (distV-distP <= 0)
                            {                        
                                AreaAttributes *nattrib = vertexCloneAttribute(inside, attrib);
                                if (nattrib)
                                {
                                    nattrib->v2 = inside;
                                    inside->flags |= GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;
                                }
                            }
                        }
                    }
                    attrib = (AreaAttributes*)attrib->next;
                }
            }            
            
            DiffEPoints(line,connected->x,in->x);
            
            if (0 == vertexLinePlaneIntersection(pinch, p, normal, in->x, line))
            {            
                // New pinch points for the connections
                Vertex *pinch_vertex = meshAddPoint(mesh, pinch);
                if (pinch_vertex)
                {                
                    err = vertexAddConnection(inside,pinch_vertex);
                }
                else
                {
                    err = -1;
                }                
            }
            else
            {
                err = -1;
            }
        }   
        else
        {
            // This connection is inside
            connected->flags |= GM_GOBJECT_VERTEX_FLAG_MCONNECTED;
        }
     }
     
     if (NULL == inside)
     {
        // No new inside needed because all connections are remaining inside
         
        Vertex *nin = meshAddPoint(mesh,in->x);
        
        if (out) // In case we got an out vertex we pinch the surface
        {
            // Add the pinch cut of the original in point with the plane too
            DiffEPoints(line,out->x,in->x);
            
            if (0 == vertexLinePlaneIntersection(pinch, p, normal, in->x, line))
            {
                Vertex *vpinch = meshAddPoint(mesh, pinch);
                if (vpinch)
                {
                    vertexAddConnection(vpinch,nin);
                }
                else
                {
                    err = -1;
                }
            }
            else
            {
                err = -1;
            }         
        }
     }
     
    for (int i = 0; (i < in->number_of_connections); i++)
    {
        Vertex *connected = in->connections[i];
        
        // Add all inside connections
        if (GM_GOBJECT_VERTEX_FLAG_MCONNECTED & connected->flags)
        {
            // These connections only need to be copied in case we got new inside vertex
            if (inside)
            {
                vertexAddConnection(inside,connected);
            }
            
            connected->flags &= ~GM_GOBJECT_VERTEX_FLAG_MCONNECTED;
        }
    }

#ifdef _DEBUG_INTERS
    if (inside)
    {
        Vertex *last_connected = NULL;
        for (int i = 0; (i < inside->number_of_connections); i++)
        {
            Vertex *connected = inside->connections[i];
            // This means we got new mesh point, propably a pinch point
            if (OBJ_MESH == objectGetType(connected->parent))
            {
                if (0 == connected->number_of_connections)
                {
                    if (last_connected)
                    {
                        // vertexAddConnection(last_connected,connected);
                        char s1[GM_VERTEX_BUFFER_SIZE],s2[GM_VERTEX_BUFFER_SIZE];
                        LOG("connect?: %s <-> %s\n",
                               vertexToString(last_connected,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE),
                               vertexToString(connected,s2,sizeof(s2),GM_V2S_FLAGS_INDEX_VERBOSE)
                        );
                        // gmathTriangleArea(last_connected,inside,connected);
                    }
                    last_connected = connected;
                }
            }        
        }
    }
#endif    
    
     return err;
}

Mesh *gmathIntersect(Solid *solid, flag_type mask, const EPoint* normal, const EPoint *p, Mesh *mesh)
{
    if (solid && mesh)
    {
        meshClear(mesh);

        char s1[GM_VERTEX_BUFFER_SIZE];
        
#ifdef _DEBUG        
        char s2[GM_VERTEX_BUFFER_SIZE];
        char s3[GM_VERTEX_BUFFER_SIZE];
        char s4[GM_VERTEX_BUFFER_SIZE];
        LOG("Intersection of [%s] with plane (%s,%s). Result [%s]\n",
            vertexPath((GObject*)solid,s1,sizeof(s1)),
            EPoint3ToString(normal,s2,sizeof(s2)),
            EPoint3ToString(p,s3,sizeof(s3)),
            vertexPath((GObject*)mesh,s4,sizeof(s4))
           );
#endif        
        
        mesh->flags &= ~GM_FLAGS_MASK_USER_FLAGS;
        mesh->flags |= (solid->flags & GM_FLAGS_MASK_USER_FLAGS);
        
        const long double distP = vertexScalarProduct(normal,p);
        int in = 0;
        Vertex *last = NULL;

        Iter iter = gobjectCreateIterator((CObject*)solid,solid->triang_filter);
        for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
        {
            Vertex *vertex = (Vertex*)isGObject(OBJ_VERTEX,obj);
            if (vertex)
            {
                const long double distV = vertexScalarProduct(vertex->x,normal);
                
    #ifdef _DEBUG_INTERS
                LOG("Inspecting [%s].parent.flags=%s\n",
                    vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE),
                    vertexFlagsToString((GObject*)vertex->parent,s2,sizeof(s2))
                );
    #endif
                
                if (vertex->parent)
                {
                    if (mask != (vertex->parent->flags & mask))
                    {
                        ERROR("Not filtered vertex [%s]\n",vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_INDEX_VERBOSE));
                    }
                }
                
                if (distV-distP <= 0)
                {
                    // In ...
                    
                    if ((0 == in) && last)
                    { 
                        // Cut to outside
                        
                        if (_gmathAddCrossingVertexToMesh(mesh, last, vertex, normal, p))
                        {
                            ERROR1("Failed to add crossing vertex to mesh.\n");
                        }                               
                    }
                    else 
                    {
                        vertexAddConnection(vertex,last);
                        
                        // Add the inside vertex and check all connections
                        if (_gmathAddCrossingVertexToMesh(mesh, NULL, vertex, normal, p))
                        {
                            ERROR1("Error: Failed to add vertex to mesh.\n");
                        }                    
            
                    }
                    
                    in = 1;
                }
                else
                {
                    // Out ...
                    
                    if ((1 == in) && last)
                    {    
                        // Cut to inside
                        
                        if (_gmathAddCrossingVertexToMesh(mesh, vertex, last, normal, p))
                        {
                            ERROR1("Failed to add crossing vertex to mesh.\n");
                        }                                                   
                    }
                    
                    in = 0;
                }
                            
                last = vertex;
            }
        }
        
        mesh->flags |= GM_GOBJECT_FLAG_TRIANGULATED;       
 
        gmathTriangFinalize((CObject*)mesh, mask);
                
        return mesh;
    }
    
    return NULL;
}

int gmathFaceNormalFromParam(EPoint *normal, EPoint *p, const EPoint *v1, const EPoint *v2, const EPoint *v3)
{
    EPoint g1[3], g2[3], g3[3];
    EPoint ng1[3], ng2[3], ng3[3];
        
    DiffEPoints(g1,v3,v2);
    DiffEPoints(g2,v3,v1);
    DiffEPoints(g3,v1,v2);
    
    vertexNormalize(ng1,g1);
    vertexNormalize(ng2,g2);
    vertexNormalize(ng3,g3);
    
    double sv1v2 = fabsl(vertexScalarProduct(ng1,ng2));
    double sv2v3 = fabsl(vertexScalarProduct(ng2,ng3));
    double sv1v3 = fabsl(vertexScalarProduct(ng1,ng3));
    
    const EPoint *px = v3;
    
    if (sv2v3 < sv1v2)
    {
        px = v1;
    }

    if (sv1v3 < sv2v3)
    {
        px = v2;
    }

    vertexMakeNorm(normal, g1,g2,g3);

#ifdef _DEBUG_TRIANG    
    char cstr[50],sstr[50];
    LOG("Norm=%s, p=%s\n",EPoint3ToString(normal,cstr,sizeof(cstr)),
        EPoint3ToString(px,sstr,sizeof(sstr)));
#endif
    
    if (p)
    {
        EPoint e1[3],e2[3];
        
        if (0 == vertexSpawnSurface(normal, px, e1, e2))
        {            
            EPoint x1[3],x2[3],x3[3],x[3];

            DiffEPoints(x,v1,px);            
            x1[0] = vertexScalarProduct(x,e1);
            x1[1] = vertexScalarProduct(x,e2);
            x1[2] = 0;

            DiffEPoints(x,v2,px);            
            x2[0] = vertexScalarProduct(x,e1);
            x2[1] = vertexScalarProduct(x,e2);
            x2[2] = 0;

            DiffEPoints(x,v3,px);            
            x3[0] = vertexScalarProduct(x,e1);
            x3[1] = vertexScalarProduct(x,e2);
            x3[2] = 0;
            
            EPoint gx2[3];
            EPoint gx3[3];
            
            DiffEPoints(gx3,x1,x2);
            DiffEPoints(gx2,x3,x1);
            
            EPointExtend(gx2,0.5,gx2);
            EPointExtend(gx3,0.5,gx3);

            
            EPoint c2[3];
            AddEPoints(c2,x1,gx2);
            DiffEPoints(c2,c2,x2);

            EPoint c3[3];
            AddEPoints(c3,x2,gx3);
            AddEPoints(c3,c3,x3);
            
            
            double det = vertexDet2(c2,c3);
            
            if (fabs(det) > 0.001)
            {
                EPoint b[3],c[3];
                DiffEPoints(b,x2,x3);
                
                double dj = vertexDet2(b,c3)/det;
                
                EPointExtend(c,dj,c2);
                
                AddEPoints(p,px,c);
#ifdef _DEBUG_TRIANG                    
                char cstr[50],sstr[50];
                LOG("Center: dj=%.3f:%s -> %s \n",dj,
                        EPoint3ToString(c,sstr,sizeof(sstr)),
                        EPoint3ToString(p,cstr,sizeof(cstr)));
#endif                
                
                return 0;
            }
        }
    }
    
    return -1;
}


// In> v1, v2, v3 (getting some flags)
// Out< normal, p
int gmathAreaNormal(Vertex *v2, EPoint *normal, EPoint *p, Vertex *v1, Vertex *v3)
{    
    if (v2 && v2->parent)
    {
        Polygon *poly = (Polygon*)isGObject(OBJ_POLYGON,v2->parent);
        if (poly && (0 == (poly->flags & GM_GOBJECT_FLAG_POLY_ABSOLUT)))
        {
            // We need absolute coordinates here
            return -1;
        }
        
        v2->flags &= ~GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;
        
        const EPoint *x1 = v1->x;
        const EPoint *x2 = v2->x;
        const EPoint *x3 = v3->x;
        
        EPoint g1[3], g2[3], g3[3];
                
        DiffEPoints(g1,x2,x3);
        DiffEPoints(g2,x3,x1);
        DiffEPoints(g3,x1,x2);

#ifdef _DEBUG_NORM                                    
        EPoint t[3];
        
        AddEPoints(t,g1,g2);
        AddEPoints(t,t,g3);
#endif
        
        if (NULL == vertexMakeNorm(normal,g1,g3,g2))
        {
            EPoint vp[3];
            vertexVectorProduct(vp,g1,g3);
            double area = vertexNorm(vp);

            char str1[GM_VERTEX_BUFFER_SIZE],str2[GM_VERTEX_BUFFER_SIZE],str3[GM_VERTEX_BUFFER_SIZE];

#ifdef _DEBUG_NORM                    
            DiffEPoints(vp,v1->x,v3->x);                    
            double dist = vertexNorm(vp);
            
            LOG("Normale failed for v1=%s, v2=%s, v3=%s\n"
                            "for |t|=%.3Lf, area=%.3f, |g1-g3|=%.3f\n",
                            vertexToString(v1, str1, sizeof(str1), GM_V2S_FLAGS_DEBUG),
                            vertexToString(v2, str2, sizeof(str2), GM_V2S_FLAGS_DEBUG),     
                            vertexToString(v3, str3, sizeof(str3), GM_V2S_FLAGS_DEBUG),
                            vertexNorm(t),area,dist);
#endif                    
            if (0.0001 > fabs(area))
            {
                ERROR("Error: empty triangle at v1=%s, v2=%s, v3=%s\n",
                            vertexToString(v1, str1, sizeof(str1), GM_V2S_FLAGS_DEBUG),
                            vertexToString(v2, str2, sizeof(str2), GM_V2S_FLAGS_DEBUG),     
                            vertexToString(v3, str3, sizeof(str3), GM_V2S_FLAGS_DEBUG));
                
                v1->flags |= GM_GOBJECT_VERTEX_FLAG_INVALID;
                v3->flags |= GM_GOBJECT_VERTEX_FLAG_INVALID;                                
            }
            
            v2->flags |= GM_GOBJECT_VERTEX_FLAG_INVALID;
            
            return -1;
        }
        
        EPoint c1[3], c2[3], c3[3];
        
        EPointExtend(c1,0.5,g1);  // p = v3
        AddEPoints(c1,x3,c1);
        DiffEPoints(c1,c1,x1);
        
        EPointExtend(c2,0.5,g2);
        AddEPoints(c2,x1,c2);
        DiffEPoints(c2,c2,x2);
        
        EPointExtend(c3,0.5,g3);
        AddEPoints(c3,x2,c3);
        DiffEPoints(c3,c3,x3);
        
#ifdef _DEBUG_TRIANG                    
        char cstr[40],gstr[40];
        LOG(" det(c1,c2,c3)=%.4f, t=%s, c2=%s, (p1=P%i.V%i,p2=P%i.V%i,p3=P%i.V%i); ",
                vertexDet3(c1,c2,c3),
                EPoint3ToString(t,cstr,sizeof(cstr)),
                EPoint3ToString(c2,gstr,sizeof(gstr)),
                v1->parent->index,v1->index,
                v2->parent->index,v2->index,
                v3->parent->index,v3->index);
#endif                
        EPoint p1[3],p2[3];
        
        if ((0 == vertexLineIntersection(p1, c1, x1, c3, x3) &&
            (0 == vertexLineIntersection(p2, c1, x1, c2, x2))))
        {
            EPoint gx[3];
            DiffEPoints(gx,p2,p1);
            
            EPointExtend(p,0.5,gx);
            AddEPoints(p,p,p1);
                            
            v2->flags |= GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;

            return 0;
        }
    }
    
    return -1;
}


int gmathRemoveDoubles(Solid *solid)
{
    for(Polygon *poly = vertexSolidPolygonIterator(solid, NULL);poly;poly = vertexSolidPolygonIterator(solid, poly))
    {
        Vertex *last = NULL;
        for (Vertex *v = vertexPolygonIterator(poly, NULL);v;)
        {
            Vertex *next = NULL;
            if (last)
            {
                EPoint diff[3];
                
                DiffEPoints(diff,v->x,last->x);
                
                long double d = vertexNorm(diff);
                
                if (fabsl(d) < 0.0001)
                {
                    last->next = v->next;
                    
                    if (v == poly->last)
                    { 
                        poly->last = last;
                    }
                    
                    next = vertexPolygonIterator(poly , v);
                    vertexDestroy(v);
                }
            }
            
            last = v;
            if (next)
            {
                v = next;
                continue;
            }
            
            v = vertexPolygonIterator(poly, v);
        }
    }
    
    return 0;
}

EPointL gmathPolygonArea(const Polygon *poly
#ifdef _DEBUG_AREA
, CObject *object
#endif    
)
{
    if (isObject(poly))
    {
        long double area_sum = 0;

        const Vertex *v0 = poly->first;
        const Vertex *vn = poly->last;
        
        if (isObject(v0) && isObject(vn))
        {
            EPoint g[3];
            DiffEPoints(g, vn->x, v0->x);
            
            EPointExtend(g,0.5,g);
            
            EPoint center[3];
            AddEPoints(center,v0->x,g);

#ifdef _DEBUG_AREA
            LOG("Center dist v0=%g, vn=%g (|g|=%Lg)\n",
                vertexSphereDistance(center,v0->x),
                vertexSphereDistance(center,vn->x),vertexNorm(g));
            
            char s1[GM_VERTEX_BUFFER_SIZE];
            char value_name[GM_VERTEX_BUFFER_SIZE];
            snprintf(value_name,sizeof(value_name),"center(%s)",vertexPath((GObject*)poly,s1,sizeof(s1)));
            Value *center_val = meaAddVector(meaGetMea(object, MEA_DEBUG), value_name, center, NULL, MeaUnit_None);
            meaSetColor(center_val, MEA_DEBUG_COLOR);
#endif
            
            const Vertex *last = NULL;
            for (const Vertex *v = vertexConstPolygonIterator(poly, NULL);v; v = vertexConstPolygonIterator(poly, v))
            {
                if (last)
                {
                    EPoint g1[3],g2[3];
                    
                    DiffEPoints(g1,last->x,center);
                    DiffEPoints(g2,v->x,center);
                    
                    EPoint nv[3];
                    vertexVectorProduct(nv, g1, g2);
                    
#ifdef _DEBUG_AREA
                    LOG("Center dist vi=%g, |g1|=%Lg, |g2|=%Lg\n",
                        vertexSphereDistance(center,v->x),vertexNorm(g1),vertexNorm(g2));
#endif
                    
                    double area = vertexNorm(nv)/2.0;
                    
                    area_sum += area;
                }
                
                last = v;
            }
        }
        
        return area_sum;
    }
    
    return 0;
}

int gmathRemoveAttributes(CObject *object)
{
    if (object)
    {
        object->flags &= ~GM_GOBJECT_FLAG_TRIANGULATED;

#ifdef _DEBUG        
        char s1[GM_VERTEX_BUFFER_SIZE];
        LOG("Removing attributes from [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
#endif
        
        Iter iter = gobjectCreateIterator(object,0);
        for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
        {
            Vertex *v = (Vertex*)isGObject(OBJ_VERTEX,obj);
            if (v)
            {
                // LOG("Removing from [%s]\n",vertexPath(obj,s1,sizeof(s1)));
                
                v->flags &= ~(GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID|GM_GOBJECT_VERTEX_FLAG_SEEN);
                char s1[GM_VERTEX_BUFFER_SIZE];
                
                for (AreaAttributes *attrib = v->first_attrib; isGObject(OBJ_ATTRIB,attrib) ;attrib = (AreaAttributes*)attrib->next)
                {   
                    if (1 < attrib->refCount)
                    {
                        ERROR("Clearing object [%s] with multiple references\n",vertexPath((GObject*)attrib,s1,sizeof(s1)));
                    }
                    
                    if (isObject(attrib))
                    {
                        attrib->flags = 0;
                        attrib->v1 = NULL;
                        attrib->v2 = NULL;
                        attrib->v3 = NULL;
                    }
                    else
                    {
                        FATAL("Invalid attribute at [%s]\n",vertexPath((GObject*)v,s1,sizeof(s1)));
                    }
                }
            }
        }
        return 0;
    }
    
    return -1;
}

double gmathVertexDistance(const Vertex *mvertex1, const Vertex *mvertex2)
{
    EPoint diff[3];
    const EPoint *x1 = mvertex1->x;
    const EPoint *x2 = mvertex2->x;
    
    DiffEPoints(diff,x1,x2);
    
    return vertexNorm(diff);
}



CObject *gmathCalculateGravity(const char* name, CObject *object, unsigned long color)
{
    if (object)
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        
#ifdef _DEBUG_CALC        
        LOG("Calculating gravity for [%s] \"%s\"\n",vertexPath((GObject*)object,s1,sizeof(s1)),object->name);
#endif        
        Mea *gravity = NULL;

        EPointL vector[3] = { 0,0,0 };
        long double mass_sum_max = 0;
        long double mass_sum_min = 0;
        int vcount = 0;
        int acount = 0;
        int atcount = 0;
        
        CIter iter = gobjectCreateConstIterator(object,0);
        for (const GObject *obj = gobjectConstIterate(iter); obj; obj = gobjectConstIterate(iter))
        {
            const Vertex *vertex = (const Vertex*)isGObject(OBJ_VERTEX,obj);
            if (vertex)
            {
#ifdef _DEBUG_CALC                        
                LOG("Checking vertex [%s]\n",vertexPath((GObject*)vertex,s1,sizeof(s1)));
#endif                
                
                const AreaAttributes *attrib = vertex->first_attrib;
                while(attrib)
                {
                    if (attrib->flags & GM_ATTRIB_FLAG_VALID)
                    {
                        const long double area = (((long double)attrib->area1) + ((long double)attrib->area2)) / 2.0L;
                        long double mass = 0;
                        
                        if (0 <= area)
                        {
                            // given here mass of a triangle is proportional to its area
                            mass = area / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M); // in Kg
                        }
                        else
                        {
                            ERROR("Found invalid area %.3Lf in [%s]\n",area,vertexPath((GObject*)vertex,s1,sizeof(s1)));
                        }
                        
                        EPointL mp[3];
                        
                        mass_sum_max += mass;
                        EPointExtend(mp,mass,attrib->normale.p);
                        AddEPoints(vector,vector,mp);
                        
                        if (NULL == gravity)
                        {
                            // Create gravity here, beacuse we realy need one now.
                            
                            gravity = meaGetMea(object, name);
                        }
                        vcount++;
                    }
                    acount++;
                    attrib = (AreaAttributes*)attrib->next;
                }
                
                atcount++;
            }
        } 
        
        EPoint pos[3];
        EPointExtend(pos,1.0/mass_sum_max,vector);

        long double grav = mass_sum_max;

        if (gravity)
        {         
            MeaUnit unit = MeaUnit_KiloGram;         
            
            if (grav > 5000.0)
            {
                grav /= 1000.0;
                
                unit = MeaUnit_Ton;         
            }
            else
            {
                if (grav <= 10)
                {
                    unit = MeaUnit_Gram;         
                    grav *= 1000;
                }
            }

            EPoint dir[3] = { 0, -grav, 0 };
            
            Value *center_of_mass = meaAddVector(gravity, "Center of mass", pos, dir, unit);
            meaSetColor(center_of_mass,color);

            snprintf(center_of_mass->description,sizeof(center_of_mass->description),"Center of mass of \"%s\"",object->name);

            Value *mass = meaAddScalar(gravity, "Mass", mass_sum_max, mass_sum_min, unit);

            snprintf(mass->description,sizeof(mass->description),"Mass of hull material of \"%s\" not mass of volume.",object->name);
#ifdef _DEBUG_CALC                    
            char s2[GM_VERTEX_BUFFER_SIZE];     
            LOG("Mass of [%s] = |%s| = %.3Lf %s (%s),\n Material: \"%s\"\n",
                vertexPath((const GObject*)object,s1,sizeof(s1)),
                EPoint3ToString(pos,s2,sizeof(s2)),
                grav,
                meaUnitToString(unit),
                mass?mass->description:UNKNOWN_SYMBOL,
                object->material?object->material->name:UNKNOWN_SYMBOL
                );        
#endif            
        }
        else
        {
            ERROR("Could not create gravity on [%s] (%i,%i,%i)\n",vertexPath((const GObject*)object,s1,sizeof(s1)),vcount,acount,atcount);
        }
        
        
        return (CObject*)gravity;
    }
    
    return NULL;
}

CObject *gmathCalculateBouyancy(const char* name, CObject *object, unsigned long color)
{
    (void) color;

    if (object)
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        
#ifdef _DEBUG_CALC                
        LOG("Calculating bouyancy for [%s] \"%s\"\n",vertexPath((GObject*)object,s1,sizeof(s1)),object->name);
#endif
        
        Mea *bouyancy = NULL;

        // EPointL vector[3] = { 0,0,0 };
        // long double bouyancy_sum_max = 0;
        // long double bouyancy_sum_min = 0;
        int pressure_vector_count = 0;
        int acount = 0;
        int atcount = 0;
        EPoint earth_center_dir[3] = GM_EARTH_CENTER;
        EPointL bouyoncy_vector[3] = GM_ZERO_VECTOR;
        
        long double bouyoncy_force = 0;
        
        CIter iter = gobjectCreateConstIterator(object,0);
        for (const GObject *obj = gobjectConstIterate(iter); obj; obj = gobjectConstIterate(iter))
        {
            const Vertex *vertex = (const Vertex*)isGObject(OBJ_VERTEX,obj);
            if (vertex)
            {
                const AreaAttributes *attrib = vertex->first_attrib;
                while(attrib)
                {
                    if (attrib->flags & GM_ATTRIB_FLAG_VALID)
                    {
                        EPoint draft_dir[3], p[3];

                        // Calculate water pressure by its draft
                        EPointExtend(p,-1.0,attrib->normale.p);
                        // The draft toward the center of the earth
                        vertexProject(draft_dir, earth_center_dir, p);
                        
                        // Draft of the triangle (depth below waterline)
                        const EPoint draft = vertexNorm(draft_dir);
    #ifdef _DEBUG_BOUYANCY
                        LOG("Draft dir: %s\n",EPoint3ToString(draft_dir,s1,sizeof(s1)));
    #endif                    

                        const long double area = (((long double)attrib->area1) + ((long double)attrib->area2)) / 2.0L;
                        
                        // given the water pressure is proportional to the draft and the triangle area???                    
                        EPoint pressure[3];                    
                        EPointExtend(pressure,(-draft*area),attrib->normale.normal);

    #ifdef _DEBUG_BOUYANCY
                        LOG("pressure dir: %s\n",EPoint3ToString(pressure,s1,sizeof(s1)));
    #endif                    
                        
                        bouyoncy_force += (draft*area);
                        AddEPoints(bouyoncy_vector,bouyoncy_vector,pressure);
                        //AddEPoints(vector,vector,attrib->normale.p);
                        pressure_vector_count++;
                        
                        if (NULL == bouyancy)
                        {
                            // Create gravity here, beacuse we realy need one now.
                            
                            bouyancy = meaGetMea(object, name);
                        }
                    }
                    acount++;
                    attrib = (AreaAttributes*)attrib->next;
                }
                
                atcount++;
            }
        } 
        
        // long double bou = vertexNormL(bouyoncy_vector);

        EPointExtend(bouyoncy_vector,1.0/bouyoncy_force,bouyoncy_vector);
        
        if (bouyancy)
        {         
            MeaUnit unit = MeaUnit_Newton;         
            
            if (bouyoncy_force > 5000.0)
            {
                bouyoncy_force /= 1000.0;
                
                unit = MeaUnit_KNewton;         
            }
            else
            {
                if (bouyoncy_force <= 10)
                {
                    unit = MeaUnit_mNewton;         
                    bouyoncy_force *= 1000;
                }
            }

            EPoint dir[3] = GM_ZERO_VECTOR;
            EPoint beoun[3];
            
            CopyEPoint(beoun,bouyoncy_vector);
            
            Value *material_bouyancy = meaAddVector(bouyancy, "Bouyancy", beoun, dir, unit);
            meaSetColor(material_bouyancy,color);

            snprintf(material_bouyancy->description,sizeof(material_bouyancy->description),"Center of bouyancy of \"%s\"",object->name);

            Value *mass = meaAddScalar(bouyancy, "Bouyancy", bouyoncy_force, bouyoncy_force, unit);

            snprintf(mass->description,sizeof(mass->description),"Bouyancy of \"%s\".",object->name);
            
#ifdef _DEBUG_CALC                    
            char s2[GM_VERTEX_BUFFER_SIZE];     
            LOG("Bouyancy of [%s] = |%s| = %.3Lf %s (%s),\n Material: \"%s\"\n",
                vertexPath((const GObject*)object,s1,sizeof(s1)),
                EPoint3ToString(beoun,s2,sizeof(s2)),
                bouyoncy_force,
                meaUnitToString(unit),
                mass?mass->description:UNKNOWN_SYMBOL,
                object->material?object->material->name:UNKNOWN_SYMBOL
                );        
#endif            
        }
        else
        {
            ERROR("Could not create bouyancy on [%s] (%i,%i,%i)\n",vertexPath((const GObject*)object,s1,sizeof(s1)),pressure_vector_count,acount,atcount);
        }
        
        
        return (CObject*)bouyancy;
    }
     
    return NULL;
}

CObject *gmathCalculateAttributes(const char* name, CObject *object, unsigned long color)
{
    (void) color;
    (void)name;
    
    char s1[GM_VERTEX_BUFFER_SIZE];
    if (object)
    {
#ifdef _DEBUG_CALC                
        LOG("Calculating %s for [%s] \"%s\"\n",name,vertexPath((GObject*)object,s1,sizeof(s1)),object->name);
#endif
        
        long double area_max = 0;
        long double area_min = 0;

        long double volume_max = 0;
        long double volume_min = 0;
        
        int err = gmathObjectVolume(object, &volume_max, &volume_min, &area_max, &area_min);

        
        double width,length,height;
        
        vertexGetCObjectSize(object, &width, &length, &height);

        const long double bbvolume = width*length*height;
        
        long double cylinder_volume = 0;
        
        Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
        if (solid)
        {
            long double max_poly_area = 0;
#ifdef _DEBUG_AREA                                
            const Polygon *max = NULL;
#endif            
            for (const Polygon *poly = vertexSolidOriginalPolygonIterator(solid, NULL); 
                NULL != poly; 
                poly = vertexSolidOriginalPolygonIterator(solid, poly))
            {
                if (poly && (solid->triang_filter == (poly->flags & solid->triang_filter)))
                {
#ifdef _DEBUG_AREA                    
                    long double area = gmathPolygonArea(poly,object);
#else                    
                    long double area = gmathPolygonArea(poly);
#endif                    
                    if (area > max_poly_area)
                    {
                        max_poly_area = area;
#ifdef _DEBUG_AREA                                                        
                        max = poly;
#endif                        
                    }
#ifdef _DEBUG_AREA                                        
                    LOG("Poly area of [%s] = %Lg\n",vertexPath((const GObject*)poly,s1,sizeof(s1)),area);
#endif                    
                }
            }

            cylinder_volume = max_poly_area * length;
#ifdef _DEBUG_AREA
            LOG("Max Poly area of [%s] = %Lg (length=%g)\n",vertexPath((GObject*)max,s1,sizeof(s1)),max_poly_area,length);
#endif            
        }
        
        if (0 == err)
        {       
            Mea *area_mea = meaGetMea(object, GMATH_MEA_ATTRIBUTES_NAME);
            if (area_mea)
            {
                Value *area_value = meaAddScalar(area_mea, "Area", area_max, area_min, MeaUnit_SquarePixel);
                snprintf(area_value->description,sizeof(area_value->description),"Area of outer surface of \"%s\".",object->name);

                Value *volume_value = meaAddScalar(area_mea, "Volume", volume_max, volume_min, MeaUnit_CubicPixel);
                snprintf(volume_value->description,sizeof(volume_value->description),"Volume of object \"%s\".",object->name);
                
                Value *bbvol = meaAddScalar(area_mea, "BBVolume", bbvolume, bbvolume, MeaUnit_CubicPixel);
                snprintf(bbvol->description,sizeof(bbvol->description),"Volume of bounding box of \"%s\".",object->name);

                Value *cb_value = meaAddScalar(area_mea, "CB", volume_max/bbvolume, volume_min/bbvolume, MeaUnit_None);
                snprintf(cb_value->description,sizeof(cb_value->description),"Block coefficient of object \"%s\".",object->name);

                if (0 < cylinder_volume)
                {
                    Value *cbvol = meaAddScalar(area_mea, "CyVolume", cylinder_volume, cylinder_volume, MeaUnit_CubicPixel);
                    snprintf(cbvol->description,sizeof(cbvol->description),"Volume of prismatic cylinder of \"%s\".",object->name);
                    
                    Value *cp_value = meaAddScalar(area_mea, "Cp", volume_max/cylinder_volume, volume_min/cylinder_volume, MeaUnit_None);
                    snprintf(cp_value->description,sizeof(cp_value->description),"Prismatic coefficient of object \"%s\".",object->name);
                }
                
                if (object->material)
                {
                    if (MeaUnit_None != object->material->densityUnit)
                    {
                        MeaUnit unit = MeaUnit_None;
                        double density_min = vertexGetMinDensitey(object->material);
                        double density_max = vertexGetMaxDensitey(object->material);
                        
                        // Volume in m³
                        double vol_max = volume_max / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                        double vol_min = volume_min / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                        
                        if (MeaUnit_KiloPerCubicDeciMeter == object->material->densityUnit)
                        {
                            // Density in Kg/m³
                            density_min = density_min * 10 * 10 * 10;
                            density_max = density_max  * 10 * 10 * 10;                        
                            
                            // Result is in Kg
                            unit = MeaUnit_KiloGram;
                        }
                        else
                        if (MeaUnit_KiloPerCubicMiliMeter == object->material->densityUnit)
                        {
                            // Density in Kg/m³
                            density_min = density_min / 100 / 100 / 100;
                            density_max = density_max  / 100 / 100 / 100;                        
                            
                            // Result is in Kg
                            unit = MeaUnit_KiloGram;                        
                        }
                        else
                        if (MeaUnit_KiloPerCubicMeter == object->material->densityUnit)
                        {
                            // Result is in Kg
                            unit = MeaUnit_KiloGram;                                                
                        }
                        else
                        {
                            ERROR("Unknown unit \"%s\" for displacement calculation\n",meaUnitToString(object->material->densityUnit));                        
                        }
                        
                        if (MeaUnit_KiloGram == unit)
                        {
                            double displacement_max = vol_max * density_max;
                            double displacement_min = vol_min * density_min;
                            
                            Value *displ = meaAddScalar(area_mea, "Displacement", displacement_max, displacement_min, unit);
                            snprintf(displ->description,sizeof(displ->description),"Displacement of \"%s\" with \"%s\".",object->name,object->material->name);                        
                            
                            unit = MeaUnit_None;
                            double thick_max = vertexGetMaxThick(object->material);
                            double thick_min = vertexGetMinThick(object->material);
                            if (MeaUnit_MiliMeter == object->material->thickUnit)
                            {
                                // Thick in m
                                thick_max = thick_max / 1000;
                                thick_min = thick_min / 1000;                                                    
                                unit = MeaUnit_KiloGram;
                            }
                            else
                            if (MeaUnit_Meter == object->material->thickUnit)
                            {
                                // Thick in m
                                unit = MeaUnit_KiloGram;
                            }                        
                            else
                            {
                                ERROR("Unknown unit \"%s\" for weight calculation\n",meaUnitToString(object->material->thickUnit));                                                    
                            }

                            
                            if (MeaUnit_KiloGram == unit)
                            {                        
                                // Area in m²
                                double a_max = area_max / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);
                                double a_min = area_min / (GMATH_PIXEL_PER_M*GMATH_PIXEL_PER_M);

                                // Weight in Kg
                                double weight_max = a_max * thick_max * density_max;
                                double weight_min = a_min * thick_min * density_min;
                                
                                Value *weight = meaAddScalar(area_mea, "Weight", weight_max, weight_min, unit);
                                snprintf(weight->description,sizeof(weight->description),"Weight of structure \"%s\" with \"%s\" thick [%.3f,%.3f] mm.",
                                        object->name,object->material->name,thick_min/1000.0,thick_max/1000.0);
                            }
                        }
                    }
                }
                
                return (CObject*)area_mea;
            }
        }
    }
     
    ERROR("Could not create attributes on [%s]\n",vertexPath((const GObject*)object,s1,sizeof(s1)));
    
    return NULL;
}

static int median_compare(const void *v1, const void *v2)
{
    EPoint *x1 = (EPoint*)v1;
    EPoint *x2 = (EPoint*)v2;
    
    return (*x1 < *x2) ? 1 : -1;
}

int gmathMedian(EPoint *values, int len, EPoint *median)
{
    if (values && (0 < len) && median)
    {
        qsort(values, sizeof(EPoint), len,median_compare);

        *median = values[len/2];
        
        return 0;
    }
    
    return -1;
}
