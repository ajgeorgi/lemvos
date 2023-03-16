#include <stdio.h>

/* Model creator for test for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "configuration.h"
#include "creator_test.h"
#include "shapes.h"
#include "modelstore.h"
#include "gmath.h"

double _test_circle_radius = 500.0;
double _test_circle_distance = 50.0;
int _test_number_of_circles = 5;
double _test_alpha1 = 0;
double _test_alpha2 = 360;

const char *_test_model_name = "test";
const char *_test_model_owner = "Georgi Bootbau";

Solid *createTest1(Model *model);
Solid *createTest2(Model *model);
Solid *createTest3(Model *model);


int initTest(CObject *object)
{
    (void)object;
    
    _test_circle_radius = configDouble(configGetConfig("test.circle_radius"),_test_circle_radius);
    _test_circle_distance = configDouble(configGetConfig("test.circle_distance"),_test_circle_distance);
    _test_number_of_circles = configInt(configGetConfig("test.number_of_circles"),_test_number_of_circles);
    
    _test_alpha1 = configDouble(configGetConfig("test.alpha1"),_test_alpha1);
    _test_alpha2 = configDouble(configGetConfig("test.alpha2"),_test_alpha2);
    
    _test_model_name = configString(configGetConfig("test.model_name"),_test_model_name);
    _test_model_owner = configString(configGetConfig("test.model_owner"),_test_model_owner);
    
    return 0;
}

Model *createTest()
{
    Model *model = modelstoreCreate(_test_model_name, _test_model_owner);

    createTest1(model);
    
    return model;
}

Solid *createTest1(Model *model)
{

    Solid* solid = modelstoreCreateSolid(model, "circle test");
  
    EPoint p[3];
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    
    shapesMarker(_test_circle_radius, p, solid);

    // fprintf(stderr,"test: r = %g, d = %g\n",_test_circle_radius,_test_circle_distance);

    int start = - _test_number_of_circles /2;
    int end = _test_number_of_circles /2;
    
    for (int i = start; i < end; i++)
    {
        Polygon *poly = modelstoreCreatePolygon(solid);
        
        p[0] = 0;
        p[1] = 0;
        p[2] = i*_test_circle_distance;
        
        vertexMoveAbsPolygon(poly,p);
        
        shapesCircle(_test_circle_radius, poly, _test_alpha1, _test_alpha2);
    }

    return solid;
}

Solid *createTest2(Model *model)
{
    Solid* solid = modelstoreCreateSolid(model, "elevation_test");

    Polygon *poly = modelstoreCreatePolygon(solid);

    shapesCircle(_test_circle_radius, poly, _test_alpha1, _test_alpha2);
    
    EPoint e[3];
    e[0] = 0;
    e[1] = 0;
    e[2] = 100;

    shapesElevatePolygon(e, poly, solid);

    return solid;
}

Solid *createTest3(Model *model)
{

    Solid* solid = modelstoreCreateSolid(model, "polynom_interpolation");
  
    EPoint xi[3];

    Polygon *poly = modelstoreCreatePolygon(solid);

    xi[0] = 100;
    xi[1] = 100;
    xi[2] = 100;
    modelstoreAddToPolygon(poly,xi);

    xi[0] = 150;
    xi[1] = 0;
    xi[2] = 0;
    modelstoreAddToPolygon(poly,xi);

    xi[0] = 0;
    xi[1] = 200;
    xi[2] = 100;
    modelstoreAddToPolygon(poly,xi);

    xi[0] = 100;
    xi[1] = 00;
    xi[2] = 00;
    modelstoreAddToPolygon(poly,xi);

    xi[0] = 100;
    xi[1] = 250;
    xi[2] = 0;
    modelstoreAddToPolygon(poly,xi);

    Polygon *poly2 = modelstoreCreatePolygon(solid);

    gmathPolynomInterpolation(poly, poly2, 10);
    
    return solid;
}
