#ifndef __PROJECTION__
#define __PROJECTION__

/* Projection for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

typedef enum {
    CSystem_None = 0,
    CSystem_XY,    
    CSystem_YZ,
    CSystem_XZ
} CoordinateSystem;

void projectionInit();
void projectionInitCam();

const char* projectionGetInfo();

void projectionTiltCam(double u, double v);

void projectionPanCam(EPoint x, EPoint y);
void projectionZoomCam(EPoint increment);

void projectionSetCSystem(CoordinateSystem csystem);

VertexProjection projectionGetProjection();

int projectionProject(const EPoint *p, int* x);

int projectionDisplayToModel(int x, int y, EPoint *position, EPoint *direction);


void projectionReset();

EPoint *projectionGetVelue(int v);


#endif // __PROJECTION__