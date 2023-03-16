#ifndef __SHAPES___
#define __SHAPES___

/* Shapes for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

int initShapes(unsigned long color);

Polygon *shapesCircle(EPoint radius, Polygon *poly, double alpha1, double alpha2);
Solid *shapesMarker(EPoint size, EPoint *p, Solid *solid);

Solid *shapesElevatePolygon(EPoint *elevation, Polygon *shape, Solid* solid);

Solid *shapeCreateFace(Model *model, const char* name, const EPoint *normal, const EPoint *p, double width, double length);

#endif // __SHAPES___
