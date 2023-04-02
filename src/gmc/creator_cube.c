#include <string.h>

/* Model creator for CUBE for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "modelstore.h"

Model *createCubeModel()
{
    EPoint x[3];
    
    Model *_model = modelstoreCreate("Test model CUBE. Simple", "Copyright: Andrej Georgi @ Georgi Bootbau");

    Solid *_theSolid = modelstoreCreateSolid(_model, "CUBE");

    
  Material *_material = modelstoreCreateMaterial(NULL, "Air");

  _material->min_density = 1.5;
  _material->max_density = 1.8;
  _material->color = GVCOLOR_PINK;
  _material->min_thick = 3;
  _material->max_thick = 5;
  strncpy(_material->spec,"St37",sizeof(_material->spec));


  vertexReplaceMaterial((CObject*)_theSolid, _material);
  
    // Origin
    x[0] = 100.0;
    x[1] = 100.0;
    x[2] = 100.0;  
    vertexMoveAbsSolid(_theSolid,x);
  

    Polygon * _polygones = modelstoreCreateOriginalPolygon(_theSolid);


    x[0] = 300.0;
    x[1] = 000.0;
    x[2] = 000.0;
    
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 1);
                           
    x[0] = 300.0;
    x[1] = 000.0;
    x[2] = 000.0;

    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 2);

    x[0] = 000.0;
    x[1] = 300.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 3);

    x[0] = -300.0;
    x[1] = 000.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 4);

    x[0] = 000.0;
    x[1] = -300.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 5);

    x[0] = 000.0;
    x[1] = 000.0;
    x[2] = 300.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 6);

    x[0] = 300.0;
    x[1] = 000.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 7);

    x[0] = 000.0;
    x[1] = 300.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 8);

    x[0] = -300.0;
    x[1] = 000.0;
    x[2] = 000.0;
  
    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 9);

    x[0] = 000.0;
    x[1] = -300.0;
    x[2] = 000.0;

    modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 10);
    
  // 2. polygon
  _polygones = modelstoreCreateOriginalPolygon(_theSolid);

  // 2. polygon at Origin + (300,0,0) of Solid
  EPoint offset[3] = { 300, 0, 0};
  AddEPoints(_polygones->p,_theSolid->p,offset);

  x[0] = 000.0;
  x[1] = 000.0;
  x[2] = 300.0;

   modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 2);

  // 3. polygon
  
  _polygones = modelstoreCreateOriginalPolygon(_theSolid);
  
  // 3. polygon at Origin + (300,300,0) of Solid
  EPoint offset2[3] = { 300, 300, 0};
  AddEPoints(_polygones->p,_theSolid->p,offset2);

  x[0] = 000.0;
  x[1] = 000.0;
  x[2] = 300.0;

   modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 3);
  
  // 4. polygon
  _polygones = modelstoreCreateOriginalPolygon(_theSolid);

  // 4. polygon at Origin + (000,300,0) of Solid
  EPoint offset3[3] = { 0, 300, 0};
  AddEPoints(_polygones->p,_theSolid->p,offset3);

  x[0] = 000.0;
  x[1] = 000.0;
  x[2] = 300.0;

   modelstoreAddToPolygonOriginal(_polygones, x, GM_GOBJECT_FLAG_POLY_ORIGINAL, 4);
  
  return _model;
}
