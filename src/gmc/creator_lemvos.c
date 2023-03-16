#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* Model creator for lemvos for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "creator_lemvos.h"

#include "configuration.h"
#include "creator_test.h"
#include "shapes.h"
#include "modelstore.h"
#include "gmath.h"
#include "commen.h"
#include "modelcreator.h"

#define nextVertex(_array,_idx) ((sizeof(_array)/sizeof(_array[0])) > (_idx) ? &_array[_idx] : NULL); (_idx += 3)

#define prevVertex(_array,_idx) ((sizeof(_array)/sizeof(_array[0])) > (_idx-3) ? &_array[_idx-3] : NULL)

const char * _lemvos_model_name = LEMVOS_MODEL_NAME;
const char * _lemvos_model_owner = "Georgi Bootbau";
int _lemvos_resolution = 20.0;
double _lemvos_frame_distance = 800;
double _lemvos_water_line = 500;

static int _lemvos_calc_bv(CObject *object);

int initLemvos(CObject *object)
{
    _lemvos_model_name = configString(configGetConfig("lemvos.model_name"),_lemvos_model_name);
    _lemvos_model_owner = configString(configGetConfig("lemvos.model_owner"),_lemvos_model_owner);
     
    _lemvos_resolution = configInt(configGetConfig("lemvos.resolution"),_lemvos_resolution);    
    _lemvos_frame_distance = configDouble(configGetConfig("lemvos.frame_distance"),_lemvos_frame_distance);
    
    _lemvos_water_line = configDouble(configGetConfig("lemvos.water_line"),_lemvos_water_line);        
    
    if (object)
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,object);
        if (model)
        {
            model->triang_filter = LEMVOS_FLAG;
            for (CObject *obj = model->first; obj; obj = (CObject*)obj->next)
            {
                Solid *solid = (Solid*)isCObject(OBJ_SOLID,obj);
                if (solid && (solid->flags & LEMVOS_FLAG))
                {
                    solid->triang_filter = LEMVOS_FLAG;
                    solid->methods.triangulate = _lemvos_calc_bv;
                    LOG("Successfully initialized Solid%i of model \"%s\".\n",solid->index,model->name);
                    // solid->methods.calcAreaNormale = _lemvos_calc_areaNormal;
                }
            }
        }
    }
    modelcreatorAddSolidCalculator(LEMVOS_MODEL_NAME, "Calc BV", _lemvos_calc_bv);
    
    return 0;
}

Polygon *createQuadraticFrame(Solid *solid, EPoint *at, double height, double width, double *area, double *length_of_polygon)
{
    Polygon *poly = modelstoreCreatePolygon(solid);

    if (length_of_polygon)
    {
        *length_of_polygon = 0;
    }
    
    Vertex *last = NULL;
    const double a = height / ((width/2)*(width/2));
    for (double dx = -width/2; dx <= width/2; dx += width/_lemvos_resolution/2)
    {
        EPoint y[3] = { dx, a * dx*dx, 0 };
        
        AddEPoints(y,y,at);
        if (last && length_of_polygon)
        {
            EPoint s[3];
            DiffEPoints(s,y,last->x);
            
            *length_of_polygon += vertexNorm(s);
        }
        
        last = modelstoreAddToPolygon(poly, y);           
    }
    
    if (area)
    {
        *area = 0;
    }
    
    // Mark polygon to be an absolute frame by the GM_FLAG_USER8
    poly->flags |= (LEMVOS_FLAG|GM_GOBJECT_FLAG_POLY_ABSOLUT);
    
    return poly;
}

Polygon *createElipticFrameAbsolut(Solid *solid, EPoint *at, double radiusX, double width, double height, double height2, double *area, double *length_of_polygon)
{
    unsigned int pidx = 0;
    EPoint parray[3000]; // this is 1000 vertices!
    int err = 0;
    memset(parray,0,sizeof(parray));
    
    double flat = width - 2.0*radiusX;
    double radiusY = height - height2;
    const double delta_alpha =  (M_PI/2.0) / _lemvos_resolution;

    if (area)
    {
        // The area the frame spawns in space. Its not the area of the hull.
        // This can be used for a volume calculation
        *area = height2 * width + (M_PI * radiusY*radiusX)*8 + flat * radiusY;
    }    
    
    if (0 > (flat - 0.0001))
    {
        ERROR("flat par = %.2f too small! width and radiusX don't match at (%.1f,%.1f,%.1f).\n",
                flat,
                at[0],at[1],at[2]);
        err = 1;
        flat = 0.0;
        width = 2*radiusX;
    }
    else
    {
        if (0 > flat)
        {
            flat = 0;
        }
    }

    if (0 > (radiusY - 0.0001))
    {
        ERROR("radiusY par = %.2f too small! height and height2 don't match at (%.1f,%.1f,%.1f).\n",
                radiusY,
                at[0],at[1],at[2]);
        
        err = -1;
    }
    else
    {
        if (0 > radiusY)
        {
            radiusY = 0;
        }
    }

    if (err < 0)
    {
        return NULL;
    }
    
    // The absolut starting point of the frame polygon
    EPoint *x0 = nextVertex(parray,pidx);
    x0[0] = - width /2.0;
    x0[1] = height;
    x0[2] = 0.0;

    // The first plottet vertex of the polygon
    EPoint *x1 = nextVertex(parray,pidx);
    x1[0] = (- width /2.0);
    x1[1] = (height -height2);
    x1[2] = 0.0;

    // Center for the elipse
    EPoint c1[3]; // not stored
    c1[0] = - flat / 2.0;
    c1[1] = radiusY;
    c1[2] = 0.0;
    
    // First vertex for the elipse
    EPoint *xb1 = nextVertex(parray,pidx);
    
    // The elipse is reverse from 270°(3/2PI) to 180°(PI)
    for (double alpha = 3.0*M_PI/2.0; alpha >= M_PI; alpha -= delta_alpha)
    {
        xb1[0] = c1[0] + radiusX*sin(alpha);
        xb1[1] = c1[1] + radiusY*cos(alpha);
        xb1[2] = c1[2];
        
        xb1 = nextVertex(parray,pidx);
    }
    
    // Continue from the last point of the elipse
    EPoint *xf = prevVertex(parray,pidx);//  pidx+=3; // &parray[pidx-6];
    
    xf[0] = flat/2.0;
    xf[1] = 0;
    xf[2] = 0;
    
    // Center for the elipse
    EPoint c2[3]; // not stored
    c2[0] = flat / 2.0;
    c2[1] = radiusY;
    c2[2] = 0.0;

    // First vertex for the elipse
    EPoint *xb2 = nextVertex(parray,pidx);

    // The elipse is reverse from 180°(PI) to 90°(PI/2)
    for (double alpha = M_PI; alpha >= M_PI/2; alpha -= delta_alpha)
    {
        xb2[0] = c2[0] + radiusX*sin(alpha);
        xb2[1] = c2[1] + radiusY*cos(alpha);
        xb2[2] = c2[2];
        
        xb2 = nextVertex(parray,pidx);
    }

    // Continue from the last point of the elipse
    EPoint *xnm1 = prevVertex(parray,pidx);  // pidx+=3;
    xnm1[0] = (width /2.0);
    xnm1[1] = (height -height2);
    xnm1[2] = 0.0;

    // The absolut starting point of the frame polygon
    EPoint *xn = nextVertex(parray,pidx);
    xn[0] = width /2.0;
    xn[1] = height;
    xn[2] = 0.0;

    // Get the polygon ...
    Polygon *poly = modelstoreCreatePolygon(solid);

    if (length_of_polygon)
    {
        *length_of_polygon = 0;
    }
    
    EPoint *xim1abs = x0;
    EPoint p[3];
    for (unsigned int i = 0; i < pidx; i += 3)
    {
        EPoint *xiabs = &parray[i];         
        
        if (length_of_polygon )
        {
            EPoint xirel[3];
            DiffEPoints(xirel,xiabs,xim1abs);
            
            *length_of_polygon += vertexNorm(xirel);
            
            xim1abs = xiabs;
        }
    
        AddEPoints(p,at,xiabs);
        // Store absolute here!
        modelstoreAddToPolygon(poly, p);   
    }
    
    // Mark polygon to be an absolute frame by the GM_FLAG_USER8
    poly->flags |= (LEMVOS_FLAG|GM_GOBJECT_FLAG_POLY_ABSOLUT);
    
    return poly;
}

Solid *createHullAbsolut(Model *model, int with_center)
{
    Solid *solid = modelstoreCreateSolid(model, "Hull(V:A.2)");

    Config *config_radiusX = configGetConfig("lemvos.radiusX");
    Config *config_width = configGetConfig("lemvos.width");
    Config *config_height = configGetConfig("lemvos.height");
    Config *config_height2 = configGetConfig("lemvos.height2");
    
    int array_length = (int)configDoubleArray(CONFIG_ARRAY_LENGTH_INDEX, config_height, 0);
    LOG("Number of frames: %i\n",array_length);
    
    double max_height = 0;
    int number_of_frames = 0;
    for (int i = 0; i < 100; i++)
    {                
        double height = configDoubleArray(i, config_height, NAN);
               
        if (isnan(height))
        {
            break;
        }
            
        if (height > max_height)
        {
            max_height = height;
        }        
        
        number_of_frames++;        
    }
    
    int index = 0;
    int bow_frame = array_length/2;
    int stern_frame = -(array_length/2);
    
    const int frame_range = (bow_frame - stern_frame) + 1;
    
    EPoint p[3] = { 0,0,0 };
    Polygon *frames[frame_range];
    double areas[frame_range];
    double length_of_polygon[frame_range];
    double estimated_volume[frame_range];
    double lemvos_length = 0;
    double lemvos_beam = 0;
    
    int bow_index = 0;
    int stern_index = 0;
    
    memset(frames,0,sizeof(frames));
    memset(areas,0,sizeof(areas));
    memset(length_of_polygon,0,sizeof(length_of_polygon));
    memset(estimated_volume,0,sizeof(estimated_volume));
    
    Polygon *center = NULL;
    if (with_center)
    {
        center = modelstoreCreatePolygon(solid);
        center->flags |= GM_GOBJECT_FLAG_POLY_ABSOLUT;
    }
    
    for (int i = stern_frame; i <= bow_frame; i++)
    {                
        double radiusX = configDoubleArray(index, config_radiusX, 500);
        double width = configDoubleArray(index, config_width, 3000);
        double height = configDoubleArray(index, config_height, NAN);
        double height2 = configDoubleArray(index, config_height2, 500);
        
        if (isnan(height))
        {
            break;
        }
        
        p[0] = 0;
        p[1] = max_height - height;
        p[2] = (i*_lemvos_frame_distance);                
        
        Polygon * poly = NULL;
        // if (i != bow_frame)
        {
            fprintf(stderr,"ElipticFrame[%i]: %.1f, %.1f, %.1f, %.1f\n",i,radiusX,width,height,height2);
            poly = createElipticFrameAbsolut(solid,p, 
                        /* double radiusX, */ radiusX,
                        /* double width, */ width,
                        /* double height, */ height,
                        /* double height2 */ height2,
                        /* double *area */ &areas[index], 
                        /* double *length_of_polygon */ &length_of_polygon[index]);
        }
//         else
//         {
//             fprintf(stderr,"QuadraticFrame[%i]: %.1f, %.1f\n",i,width,height);
//             poly = createQuadraticFrame(solid,p, 
//                         /* double width, */ width,
//                         /* double height, */ height,
//                         /* double *area */ &areas[index], 
//                         /* double *length_of_polygon */ &length_of_polygon[index]);
//         }
                        
        if (NULL == poly)
        {
            ERROR("%i: Frame failed: radiusX = %.1f, width = %.1f, height = %.1f, height2 = %.1f\n",
                    i,
                    radiusX,
                    width,
                    height,
                    height2);                    
        }
        
        estimated_volume[index] = areas[index] * _lemvos_frame_distance;

        frames[index] = poly;
        
        if (lemvos_beam < width)
        {
            lemvos_beam = width;
        }        
          
        if (center)
        {
            modelstoreAddToPolygon(center, p);
        }
        
        index++;
    }

    lemvos_length = (number_of_frames - 1) * _lemvos_frame_distance;

    Polygon *stern = NULL;
    if (number_of_frames > 0)
    {
        if (length_of_polygon[0] > length_of_polygon[number_of_frames-1])
        {
            stern_index = frames[0]->index;
            bow_index = frames[number_of_frames-1]->index;
            frames[number_of_frames-1]->flags |= LEMVOS_FLAG_BOW;
            frames[0]->flags |= LEMVOS_FLAG_STERN;            
            stern = frames[0];
        }
        else
        {
            bow_index = frames[0]->index;
            stern_index = frames[number_of_frames-1]->index;
            frames[0]->flags |= LEMVOS_FLAG_BOW;
            frames[number_of_frames-1]->flags |= LEMVOS_FLAG_STERN;            
            stern = frames[number_of_frames-1];
        }        
    }
        
    char textBuffer[1500];
    textBuffer[0] = 0;
    char text[300];

    double hullarea1 = 0;
    double polyarea2 = 0;
    double aprox_volume = 0;
    double *length_of_polygon_last = NULL;
    for (int i = 0; i < number_of_frames; i++)
    {
        const char *are_unit = "px²";
        const char *len_unit = "px";
        double frame_area = areas[i];

        // Sum of polygon areas. Not the hull area
        polyarea2 += frame_area;
        
        if (1000000.0 < frame_area)
        {
            frame_area /= 1000000.0;
            are_unit = "kpx²";
        }
        
        double poly_length = length_of_polygon[i];
        if (1000.0 < poly_length)
        {
            poly_length /= 1000.0;
            len_unit = "kpx";
        }
        
        int pidx = frames[i]?frames[i]->index:0;
        snprintf(text,sizeof(text),"F%i(Poly%i):A(P%i)=%'.3f%s, len=%'.3f%s,\n",i+1,pidx,pidx,
                 frame_area,are_unit,
                 poly_length,len_unit);
        
        commenStringCat(textBuffer,text,sizeof(textBuffer));
        
        if ((LEMVOS_FLAG|GM_GOBJECT_FLAG_POLY_ABSOLUT) == (frames[i]->flags & (LEMVOS_FLAG|GM_GOBJECT_FLAG_POLY_ABSOLUT)))
        {
            if (length_of_polygon_last)
            {
                hullarea1 += (*length_of_polygon_last + length_of_polygon[i])/2.0 * _lemvos_frame_distance;
                
                // This is not valid for the first frame. Two polygones needed for a trapez
                aprox_volume += estimated_volume[i-1];
            }
            length_of_polygon_last = &length_of_polygon[i];
        }
    }

    if (center && stern)
    {
        EPoint s[3];
        DiffEPoints(s,stern->last->x,stern->first->x);

        EPointExtend(s,0.5,s);        
        AddEPoints(s,stern->first->x,s);
        
        if (gmathVertexDistance(center->first, stern->first) < gmathVertexDistance(center->last, stern->first))
        {
            modelstoreInsertToPolygon(center, s);
        }
        else
        {
            modelstoreAddToPolygon(center,s);
        }
    }
    
    const char *are1_unit = "px²";
    const char *are2_unit = "px²";
//    const char *are3_unit = "px²";        
    const char *beam_unit = "px";
    const char *len_unit = "px";
    const char *vol_unit = "px³";

    if (1000000000.0 < aprox_volume)
    {
        aprox_volume /= 1000000000.0;
        vol_unit = "kpx³";
    }

    if (1000000.0 < hullarea1)
    {
        hullarea1 /= 1000000.0;
        are1_unit = "kpx²";
    }

    if (1000000.0 < polyarea2)
    {
        polyarea2 /= 1000000.0;
        are2_unit = "kpx²";
    }

    if (1000.0 < lemvos_length)
    {
        lemvos_length /= 1000.0;
        len_unit = "kpx";
    }

    if (1000.0 < lemvos_beam)
    {
        lemvos_beam /= 1000.0;
        beam_unit = "kpx";
    }

    /* double hullarea = gmathSolidArea(solid,(LEMVOS_FLAG|GM_GOBJECT_FLAG_POLY_ABSOLUT));

    if (1000000.0 < hullarea)
    {
        hullarea /= 1000000.0;
        are3_unit = "kpx²";
    }
*/
    snprintf(text,sizeof(text),"Sum(A(Pi))=%'.3f%s, est Vol=%'.3f%s\nHullarea=%'.3f%s , HullLen= %'.3f%s,\nbeam=%'.3f%s, stern=P%i, bow=P%i\n",
             polyarea2,are2_unit,
             aprox_volume,vol_unit,
             // hullarea,are3_unit,
             hullarea1,are1_unit,
             lemvos_length,len_unit,
             lemvos_beam,beam_unit,
             stern_index,bow_index);
    
    commenStringCat(textBuffer,text,sizeof(textBuffer));
    
    solidAddComment(solid,textBuffer,sizeof(textBuffer));
        
    fprintf(stderr,textBuffer);
    
    return solid;
}

Model *createLemvos()
{
    Model *model = modelstoreCreate(_lemvos_model_name, _lemvos_model_owner);

    Solid *hull = createHullAbsolut(model,/* with center */ 1);
    hull->flags |= LEMVOS_FLAG;
    hull->methods.triangulate = _lemvos_calc_bv;
    
    return model;
}

int _lemvos_calc_bv(CObject *object)
{
    Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
    if (solid && (solid->flags & LEMVOS_FLAG))
    {
        char s1[GM_VERTEX_BUFFER_SIZE];     
        LOG("triangulate: %s: \"%s\"\n",
                    vertexPath((GObject*)solid,s1,sizeof(s1)),solid->name?solid->name:UNKNOWN_SYMBOL);
        
        vertexMakeBBNormal(&solid->box);
        gmathSolidArea(solid, LEMVOS_FLAG);
        
        Polygon *stern = NULL;
        Polygon *center = NULL;

        for (Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
        {
            if ((LEMVOS_FLAG_STERN|LEMVOS_FLAG) == (poly->flags & (LEMVOS_FLAG_STERN|LEMVOS_FLAG)))
            {
                stern = poly;
            }
            else
            if (0 == (poly->flags & (LEMVOS_FLAG_STERN|LEMVOS_FLAG_BOW|LEMVOS_FLAG)))
            {
                center = poly;
            }            
        }
        
        if (center && stern)
        {
            Vertex *c1 = NULL;
            if (gmathVertexDistance(center->first, stern->first) < gmathVertexDistance(center->last, stern->first))
            {
                c1 = center->first;
            }
            else
            {
                c1 = center->last;
            }            
            
            if (c1)
            {
                Vertex *last = NULL;
                for (Vertex *vertex = stern->first; NULL != vertex; vertex = (Vertex*)vertex->next)
                {
                    if (last)
                    {
                        gmathTriangleArea(vertex, last, c1, 0);
                    }
                    last = vertex;
                }
                vertexAddConnection(last, c1);
            }
        }
        
        gmathTriangFinalize((CObject*)solid, LEMVOS_FLAG);

        
        return 0;
    }
    
    return -1;
}
