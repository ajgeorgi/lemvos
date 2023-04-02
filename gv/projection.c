#include <string.h>
#include <stdio.h>
#include <math.h>

/* Projection for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "projection.h"
#include "configuration.h"
#include "viewer.h"

#define CAM_DISTANCE 500
#define CAM_HEIGHT -6000
 
double _viewer_cam_distance = CAM_DISTANCE;
double _viewer_cam_height = CAM_HEIGHT;

static struct {
    EPoint h[3];
    EPoint e1[3];
    EPoint e2[3];
    EPoint n0[3];

    EPoint ie1[3];
    EPoint ie2[3];
    EPoint in0[3];
    
    EPoint distance;
    EPoint height;
    EPoint s[3];

    EPoint *axis1;
    EPoint *axis2;
    EPoint *axis3;
    
    double back_u;
    double back_v;
    
    EPoint pan_x;
    EPoint pan_y;
} _viewerCam;

CoordinateSystem _projection_csystem = CSystem_None;

void projectionInit()
{
    memset(&_viewerCam,0,sizeof(_viewerCam));
    
    projectionReset();
}

void projectionReset()
{
    if (CSystem_None == _projection_csystem)
    {
        _projection_csystem = CSystem_XY;
    }

    _viewer_cam_distance = configDouble(configGetConfig("projection.cam_distance"),CAM_DISTANCE);
    _viewer_cam_height = configDouble(configGetConfig("projection.cam_height"),CAM_HEIGHT);
    
    projectionInitCam();

    projectionSetCSystem(_projection_csystem);
}

void projectionSetCSystem(CoordinateSystem csystem)
{    
    
    switch(csystem)
    {
        case CSystem_None:
        case CSystem_XY:
            _viewerCam.axis1 = _viewerCam.e1;
            _viewerCam.axis2 = _viewerCam.e2;
            _viewerCam.axis3 = _viewerCam.n0;            
            csystem = CSystem_XY;
            break;
        case CSystem_YZ:            
            _viewerCam.axis1 = _viewerCam.e2;
            _viewerCam.axis2 = _viewerCam.n0;
            _viewerCam.axis3 = _viewerCam.e1;            
            break;
        case CSystem_XZ:            
            _viewerCam.axis1 = _viewerCam.e1;
            _viewerCam.axis2 = _viewerCam.n0;
            _viewerCam.axis3 = _viewerCam.e2;            
            break;
    }    

    _projection_csystem = csystem;

    projectionTiltCam(0,0);
}

void projectionInitCam()
{

    // fprintf(stderr,"CAM distance = %.1f\n",_viewerCam.distance );

    _viewerCam.distance = _viewer_cam_distance;   
    _viewerCam.height = _viewer_cam_height;    

    
    _viewerCam.back_u = 0;   
    _viewerCam.back_v = 0;    

    int window_width = 0;
    int window_height = 0;
    viewerGetWindowSize(&window_width, &window_height);
    
    _viewerCam.s[0] = window_width/2;
    _viewerCam.s[1] = window_height/2;
    _viewerCam.s[2] = 0;
        
    viewerInvalidate();
}


void projectionPanCam(EPoint dx, EPoint dy)
{
    _viewerCam.s[0] += dx;
    _viewerCam.s[1] += dy;
    _viewerCam.s[2] = 0;
    
    viewerInvalidate();
}

void projectionZoomCam(EPoint increment)
{
    if (_viewer_cam_distance > 0)
    {
        if (((0 < increment) && ((- _viewerCam.height - 100) > _viewerCam.distance)) ||
            ((0 > increment) && (50 < _viewerCam.distance)))
        {
            _viewerCam.distance += increment;           
            viewerInvalidate();   
        }
    }
    else
    if (_viewer_cam_distance < 0)
    {
        if (((0 > increment) && ((100 - _viewerCam.height) < _viewerCam.distance)) ||
            ((0 < increment) && (-50 > _viewerCam.distance)))
        {
            _viewerCam.distance += increment;           
            viewerInvalidate();   
        }
    }        
}

#define PR_AXIS_X (1<<0)
#define PR_AXIS_Y (1<<1)
#define PR_AXIS_Z (1<<2)
#define PR_AXIS_INV (1<<3)

void rotate(int axis, EPoint *e1, EPoint *e2, EPoint *e3, double thx, double thy)
{
    (void)thy;
    
    const EPoint cx = cos(thx);
    // const EPoint cy = cos(thy);
    const EPoint sx = sin(thx);
    // const EPoint sy = sin(thy);    
    
    const EPoint rotx[3 * 3] = {
        1,     0,    0,
        0,    cx,   sx,
        0,   -sx,   cx  
    };

    const EPoint irotx[3 * 3] = {
           1,     0,   0,
           0,    cx,   -sx,
           0,    sx,   cx 
    };

    /*
    const EPoint roty[3 * 3] = {
        cy,   0,   -sy,
        0,    1,   0,
        sy,   0,   cy  
    };

    const EPoint iroty[3 * 3] = {
           cy,  0,   sy,
           0,   1,   0,
           -sy,    0,   cy 
    };
    
    const EPoint rxy[] = {
        cy,    0,    sy,
        sx*sy, cx,  -sx*cy,
        -cx*sy, sx,  cx*cy
    };
                
    const EPoint irxy[] = {
           cy,     0,   -sy,
           sx*sy, cx,   cy*sx,
           sy*cx, -sx,  cx*cy 
    };    
    */
    
    EPoint *column[3] = { e1,e2,e3 };
    const EPoint *_row[3] = { &rotx[0], &rotx[3], &rotx[6] };
    const EPoint *_irow[3] = { &irotx[0], &irotx[3], &irotx[6] };
    const EPoint **row;
    
    if (PR_AXIS_INV & axis)
    {                
        row = _irow;
    }
    else
    {
        row = _row;
    }
    
    for (int i = 0; i < 3; i++)
    {        
        column[i][0] = row[0][i];
        column[i][1] = row[1][i];
        column[i][2] = row[2][i];
    }
}

void projectionTiltCam(double u, double v)
{
    _viewerCam.back_u += u;
    _viewerCam.back_v += v;
            
    double thx = 2*M_PI/360.0 * _viewerCam.back_u;
    double thy = 2*M_PI/360.0 * _viewerCam.back_v;
            
    rotate(PR_AXIS_X | PR_AXIS_Y, _viewerCam.axis1,_viewerCam.axis2,_viewerCam.axis3,thx,thy);
    
    _viewerCam.h[0] = _viewerCam.height * _viewerCam.n0[0] ;
    _viewerCam.h[1] = _viewerCam.height * _viewerCam.n0[1] ;
    _viewerCam.h[2] = _viewerCam.height * _viewerCam.n0[2] ;
    
    viewerInvalidate();
}


int projectionDisplayToModel(int x, int y, EPoint *position, EPoint *direction)
{
    (void)direction;
    
    const double  thx  = 2*M_PI/360.0 * _viewerCam.back_u;
    const double thy = 2*M_PI/360.0 * _viewerCam.back_v;
    
    rotate(PR_AXIS_INV | PR_AXIS_X | PR_AXIS_Y, _viewerCam.axis1,_viewerCam.axis2,_viewerCam.axis3,thx,thy);        

    EPoint camH[3];
    camH[0] = _viewerCam.height * _viewerCam.n0[0] ;
    camH[1] = _viewerCam.height * _viewerCam.n0[1] ;
    camH[2] = _viewerCam.height * _viewerCam.n0[2] ;

    int window_height = 0;
    viewerGetWindowSize(NULL, &window_height);

    EPoint p1[3] = { 0, 0, 0 };
    p1[0] = (x - _viewerCam.s[0]);
    p1[1] = ((window_height-y) - _viewerCam.s[1]);
    // p1[2] = p1[0] * _viewerCam.distance;

    AddEPoints(p1, camH, p1);
        
    EPoint p1i[3];
    p1i[0] = vertexScalarProduct(p1,_viewerCam.e1);
    p1i[1] = vertexScalarProduct(p1,_viewerCam.e2);
    p1i[2] = vertexScalarProduct(p1,_viewerCam.n0);
    
    CopyEPoint(position,p1i);
    
    rotate(PR_AXIS_X | PR_AXIS_Y, _viewerCam.axis1,_viewerCam.axis2,_viewerCam.axis3, thx, thy);        
    
    return 1;
}

int projectionProject(const EPoint *p, int* x)
{
  EPoint p1[GM_VERTEX_DIMENSION],p3[GM_VERTEX_DIMENSION];
  
  // Was wenn p = 0?
  DiffEPoints(p1,p,_viewerCam.h);
  
  p3[X_PLANE] = vertexScalarProduct(p1,_viewerCam.e1);
  p3[Y_PLANE] = vertexScalarProduct(p1,_viewerCam.e2);
  p3[Z_PLANE] = vertexScalarProduct(p1,_viewerCam.n0);

  EPoint x1 = (p3[Z_PLANE]/_viewerCam.distance); // ???
  
  /*
  if (x1*x1 < 0.0001)
  {
      viewerPrintError(0, "Projection failed! Change viewer.cam_* settings in config.\n");
  }
*/
    int window_height = 0;
    viewerGetWindowSize(NULL, &window_height);

  // Move origin to left bottom??
  x[0] = p3[X_PLANE]/x1 + _viewerCam.s[0]; 
  x[1] = window_height - (p3[Y_PLANE]/x1 +_viewerCam.s[1]);
  
  return 1;
}

VertexProjection projectionGetProjection()
{
    return projectionProject;
}

const char* projectionGetInfo()
{
    static char infoText[200];
    
    int window_width = 0;
    int window_height = 0;
    viewerGetWindowSize(&window_width, &window_height);

    /*
    EPoint p[3];
    p[0] = _viewerCam.h[0],
    p[1] = _viewerCam.h[1],
    _viewerCam.h[2],
*/

    const double len = vertexNorm(_viewerCam.n0);
    
    snprintf(infoText,sizeof(infoText),"Screen (%i,%i)\nCam dist = %.2f, Cam height = %.2f\n"
                                        "n0=|(%.2f,%.2f,%.2f)|=%.2f,e1=(%.2f,%.2f,%.2f),\ne2=(%.2f,%.2f,%.2f),\n",
             window_width,window_height,
             _viewerCam.distance,
             _viewerCam.height,
             _viewerCam.n0[0],
             _viewerCam.n0[1],
             _viewerCam.n0[2],
             len,
             _viewerCam.e1[0],
             _viewerCam.e1[1],
             _viewerCam.e1[2],
             _viewerCam.e2[0],
             _viewerCam.e2[1],
             _viewerCam.e2[2]);


             
    return infoText;
}

EPoint *projectionGetVelue(int v)
{
    if (0 == v)
    {
        return _viewerCam.h;
    }
    else
    if (1 == v)
    {
        return &_viewer_cam_distance;
    }
    else
    if (2 == v)
    {
        return &_viewer_cam_height;
    }
    
    return NULL;
}