#ifndef __GMATH__
#define __GMATH__

/* GMath for GMC 10.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


#define GMATH_MEA_GRAVITY_NAME "Mass"
#define GMATH_MEA_BOUYANCY_NAME "Bouyancy"
#define GMATH_MEA_ATTRIBUTES_NAME "Attributes"

#define GM_MATERIAL_WATER_NAME "Water"

int gmathInit();


Polygon *gmathPolynomInterpolation(Polygon *poly, Polygon *result, int resolution);

// int gmathLemvosSurface(Solid *solid, unsigned long polygon_flag_mask);
int gmathFaceNormalFromParam(EPoint *normal, EPoint *p, const EPoint *v1, const EPoint *v2, const EPoint *v3);

double gmathVertexDistance(const Vertex *mvertex1, const Vertex *mvertex2);

Mesh *gmathIntersect(Solid *solid, flag_type mask, const EPoint* normal, const EPoint *p, Mesh *mesh);

double gmathSolidArea(Solid *solid, flag_type mask);
long double gmathTriangleArea(Vertex *v1, Vertex *v2, Vertex *v3, flag_type mask);
int gmathAreaNormal(Vertex *v2, EPoint *normal, EPoint *p,  Vertex *v1,  Vertex *v3);

int gmathObjectVolume(CObject* object, EPointL *max_volume, EPointL *min_volume, EPointL *max_area, EPointL *min_area);

int gmathTriangFinalize(CObject *object, flag_type mask);

#ifdef _DEBUG_AREA
EPointL gmathPolygonArea(const Polygon *poly, CObject *object);
#else
EPointL gmathPolygonArea(const Polygon *poly);
#endif

int gmathRemoveAttributes(CObject *object);
int gmathRemoveDoubles(Solid *solid);
int gmathMeshRemoveDoubles(Mesh *mesh);
int gmathTriangulate(Mesh *mesh);

int gmathMedian(EPoint *values, int len, EPoint *median);


#endif // __GMATH__
