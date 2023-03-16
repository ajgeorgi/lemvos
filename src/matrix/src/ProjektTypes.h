#ifndef _PROJEKTTYPES_H
#define _PROJEKTTYPES_H

#include <limits.h>
#include <math.h>
#define _INFINLONG LONG_MAX
#define _INFINULONG ULONG_MAX
#define _INFINDOUBLE HUGE_VAL

// The fixpoint data type
typedef int fix;

// Mesures
typedef int           ObjectDataQuality; // Data quality in percent (0-100)
typedef unsigned long ObjectTime; // Time in milliseconds
typedef long    ObjectDist;      // Distance in mm
typedef double  ObjectAngle;     // Angle in radiant
typedef double  ObjectVelocity;  // Velocity in m/s

class ObjectVector
{
 public:

  ObjectVector()
    : theDimension(0),
      theVector((ObjectDist*)0)
    {
    };

  ObjectVector(int dim)
    : theDimension(dim)
    {
      REQUIRE(theDimension > 0);
      theVector = new ObjectDist[theDimension];
      REQUIRE(theVector);
      for (int i = 0;i< theDimension;i++)
	theVector[i] = 0;
    };

  ObjectVector(ObjectDist x1,
	       ObjectDist x2,
	       ObjectDist x3,
	       ObjectDist x4)
    : theDimension(4)
    {
      theVector = new ObjectDist[theDimension];
      theVector[0] = x1;
      theVector[1] = x2;
      theVector[2] = x3;
      theVector[3] = x4;
    };

  ~ObjectVector()
    {
      if (theVector)
	delete theVector;
    };

  const ObjectVector &operator = (const ObjectVector &v2)
  { 
    if (theDimension == 0)
    {
      REQUIRE(theVector == (ObjectDist*)0);
      REQUIRE(v2.theDimension > 0);
      theVector = new ObjectDist[v2.theDimension];
    }
    REQUIRE(v2.theDimension == theDimension);
    for(int i = 0;i < theDimension;i++)
       theVector[i] = v2.theVector[i];
    
    return *this;
  };

  const void *_base() const { return (void*)theVector; };
 
  long &operator [] (unsigned int idx)
    {
      REQUIRE(idx < theDimension);
      return theVector[idx];
    };

  long operator [] (unsigned int idx) const
    {
      REQUIRE(idx < theDimension);
      return theVector[idx];
    };

  int GetDimension() const { return theDimension; };

 protected:
  int theDimension;
  ObjectDist *theVector;
};

// An infinit distance
const ObjectDist InfinitObjectDist = _INFINLONG-1;
// An unknown distance
const ObjectDist UnknownObjectDist = _INFINLONG;

// An unknown object speed
const ObjectVelocity UnknownObjectVelocity = _INFINLONG;

// An unknown position
const ObjectVector UnknownObjectVector(_INFINLONG,_INFINLONG,_INFINLONG,_INFINLONG );

// An unknown angel
const ObjectAngle UnknownObjectAngle = _INFINDOUBLE;

// An undefined time
const ObjectTime UndefObjectTime = _INFINULONG;

#define TIMETOSECOND(_x) ((_x)*1000)

#define MIN(_a,_b) ((_a) < (_b) ? (_a) : (_b))
#define MAX(_a,_b) ((_a) > (_b) ? (_a) : (_b))
#define ABS(_a)   ((_a) < 0 ? ((-1)*(_a)) : (_a))
#define SQR(_x)   ((_x)*(_x))
#define SIGN(_x)  ((_x) < 0 ? (-1) : (+1))
#define SQRT2  M_SQRT2

#define _SET(_f,_x) ((_f) |= (_x))
#define _UNSET(_f,_x) ((_f) &= ~(_x))
#define _IS(_f,_x) ((_f) & (_x))


#define EPSILON 0.000001
#define MACHEPS 2.22045e-16 

#define DOUBLETOLONG(_x) (ABS(_x) > _INFINLONG ? (double)SIGN(_x)*_INFINLONG : (double)(_x))

#define PI M_PI

#define TORAD(_a) ((_a)*M_PI/180.0)
#define TODEG(_a) ((_a)*180.0/M_PI)

#ifndef _BOOL_DEF
#define _BOOL_DEF
enum Bool { NO = 0,YES };
#endif

typedef unsigned char Byte;

#define BOOL_NAMES(_b) ((_b) == 0) ? "NO" : ((_b) == 1) ? "YES" : "UNKNOWN"

#endif // _PROJEKTTYPES_H
