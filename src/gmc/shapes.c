#include <math.h>
#include <string.h>


/* Shapes for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "shapes.h"
#include "modelstore.h"
#include "configuration.h"
#include "aperal.h"

double _shapes_circle_precision = 20.0;
// Material _shapesTestMaterial;

int initShapes(unsigned long color)
{
    (void)color;
    
    _shapes_circle_precision = configDouble(configGetConfig("shapes.circle_precision"),_shapes_circle_precision);
/*        
    memset(&_shapesTestMaterial,0,sizeof(_shapesTestMaterial));
    strncpy(_shapesTestMaterial.name,"shape",sizeof(_shapesTestMaterial.name));
    strncpy(_shapesTestMaterial.spec,"virtual material",sizeof(_shapesTestMaterial.spec));
    _shapesTestMaterial.color = color;
    _shapesTestMaterial.refCount = 100;
  */  
    return 0;
}

Polygon *shapesCircle(EPoint radius, Polygon *poly, double alpha1, double alpha2)
{
    EPoint xi[3];    
    EPoint xi1[3] = { 0,0,0 };
    EPoint v[3];
    
    int firstDot = 1;
    
    double start = (alpha1 / 360.0) * 2* M_PI;
    double end = (alpha2 / 360.0) * 2* M_PI;         
    
    for (double alpha = start; alpha <= end; alpha += (M_PI*2.0/_shapes_circle_precision))
    {
        xi[X_PLANE] = cos(alpha) * radius;
        xi[Y_PLANE] = sin(alpha) * radius;                
        xi[Z_PLANE] = 0;  // Plane circle no extension to the Z plane

        if (0 == firstDot)
        {
            DiffEPoints(v,xi,xi1);           
        }
        else
        {
            firstDot = 0;
            CopyEPoint(v,xi);
        }
        
        modelstoreAddToPolygon(poly, v);
        
        CopyEPoint(xi1,xi);
    }
        
    return poly;
}

Solid *shapesMarker(EPoint msize, EPoint *p, Solid *solid)
{
    EPoint p1[GM_VERTEX_DIMENSION]= { -msize,0.0,0.0 };
    EPoint x2[GM_VERTEX_DIMENSION]= { 2*msize,0.0,0.0 };

    EPoint p2[GM_VERTEX_DIMENSION]= { 0,-msize,0 };
    EPoint y2[GM_VERTEX_DIMENSION]= { 0,2*msize,0 };

    EPoint p3[GM_VERTEX_DIMENSION]= { 0,0,-msize };
    EPoint z2[GM_VERTEX_DIMENSION]= { 0,0,2*msize };

    AddEPoints(p1,p1,p);
    AddEPoints(p2,p2,p);    
    AddEPoints(p3,p3,p);

    Polygon *poly1 = modelstoreCreatePolygon(solid);
    Polygon *poly2 = modelstoreCreatePolygon(solid);
    Polygon *poly3 = modelstoreCreatePolygon(solid);

    modelstoreAddToPolygon(poly1, x2);
    modelstoreAddToPolygon(poly2, y2);
    modelstoreAddToPolygon(poly3, z2);
    
    vertexMoveAbsPolygon(poly1,p1);
    vertexMoveAbsPolygon(poly2,p2);
    vertexMoveAbsPolygon(poly3,p3);
    
    return solid;
}


Polygon *shapesCopyPolygon(Solid *solid, Polygon *poly)
{
    Polygon *result = modelstoreCreatePolygon(solid);
    for (Vertex *x = poly->first; NULL != x; x = (Vertex*)x->next)
    {
        modelstoreAddToPolygon(result, x->x);
    }
    
    return result;
}

Solid *shapesElevatePolygon(EPoint *elevation, Polygon *shape, Solid* solid)
{
    (void)elevation;
    
    if (solid && shape && elevation)
    {
        EPoint s[3] = { 0,0,0 };

        Polygon *poly1 = shapesCopyPolygon(solid, shape);

        AddEPoints(s,shape->p,elevation);
        vertexMoveAbsPolygon(poly1,s);
        
        CopyEPoint(s,poly1->p);
        
        for (Vertex *x = shape->first; NULL != x; x = (Vertex*)x->next)
        {
             AddEPoints(s,s,x->x);
                                      
             Polygon *polyi = modelstoreCreatePolygon(solid);
             
             vertexMoveAbsPolygon(polyi,s);
             
             modelstoreAddToPolygon(polyi,s);
             
             
        }
    }
    
    return NULL;
}

Polygon *shapeCreateSpline(Solid *solid)
{
    (void)solid;
    
    return NULL;
}

Polygon *shapeCreateBoundingBox(Solid *solid)
{
    EPoint boundingBoxMin[3];
    EPoint boundingBoxMax[3];
    
    EPoint pi[GM_VERTEX_DIMENSION] = {0,0,0}; 
    EPoint pi_old[GM_VERTEX_DIMENSION] = {0,0,0};    
    
    for (const Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
    {    
        for (const Vertex *vertex = poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
        {          
            // p_i = x + p_(o-1) (p absolut, x relativ)
            AddEPoints(pi,vertex->x,pi_old);
        
            for (int i = 0; i < 3; i++)
            {
                if (pi[i] > boundingBoxMax[i])
                {
                    boundingBoxMax[i] = pi[i];
                }
                if (pi[i] < boundingBoxMin[i])
                {
                    boundingBoxMin[i] = pi[i];
                }
            }
            
            CopyEPoint(pi_old,pi);
        }
    }
    
    // Polygon *boundBox = modelstoreCreatePolygon(solid);
    /*
    EPoint x1[3] = boundingBoxMin;
    EPoint x2[3] = boundingBoxMin;
    EPoint x3[3] = boundingBoxMin;
    */
    // modelstoreAddToPolygon(boundBox, EPoint *v);

    return NULL;
}

Solid *shapeCreateFace(Model *model, const char* name, const EPoint *normal, const EPoint *p, double width, double length)
{
    EPoint e1[3];
    EPoint e2[3];

    if ((0 < width) && (0 < length) && (1000000 > length) && (1000000 > width))
    {    
        if (0 == vertexSpawnSurface(normal,p,e1,e2))
        {
            const double weps = fabs(1/width);
            const double leps = fabs(1/length);
            
            Solid *solid = modelstoreCreateSolid(model, name);
            // solid->material = &_shapesTestMaterial;
            
            EPoint xi[3],x1[3],y1[3];
            for (double syi = -length/2; syi <= length/2 + leps; syi += 500)
            {        
                EPointExtend(y1,syi,e1);
                AddEPoints(y1,y1,p);
                
                Polygon *polyx = modelstoreCreatePolygon(solid);
                polyx->flags |= GM_GOBJECT_FLAG_POLY_ABSOLUT;
                
                for (double sxi = -width/2; sxi <= width/2 + weps; sxi += 500)
                {
                    EPointExtend(xi,sxi,e2);
                    AddEPoints(x1,xi,y1);            
                                
                    modelstoreAddToPolygon(polyx,x1);
                }
            }
            
            CopyEPoint(solid->normal.normal,normal);
            CopyEPoint(solid->normal.p,p);
            
            solid->flags |= GM_GOBJECT_FLAG_SURFACE;
            
            return solid;
        }                
    }   
    
    ERROR("Error: Failed to spawn face (width=%.3f, length=%.3f)\n",width,length);
    
    return NULL;
}

