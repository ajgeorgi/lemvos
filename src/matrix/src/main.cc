

#include <time.h>
#include "hmatrix.h"

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    
  Matrix A(3,3);
  Matrix Q;
  Vector l;

  A(0,0) = 1 ;  A(0,1) = 0; A(0,2) = 1.140;
  A(1,0) = 1; A(1,1) = -0.344;  A(1,2) = -0.714;
  A(2,0) = 1; A(2,1) = 1.772;  A(2,2) = 0;

  
  COUT << "A(0)*A(1) = " << A.Column(0)*A.Column(1) << ENDL;
  COUT << "A(1)*A(2) = " << A.Column(1)*A.Column(2) << ENDL;
  COUT << "A(2)*A(0) = " << A.Column(2)*A.Column(0) << ENDL;

  COUT << "det A = " << A.Det() << ENDL;
  
  Matrix iA = A.Invert();

  COUT << "A^-1 = " << iA;

  COUT << "A = " << A;
  
  COUT << "A*A^-1 = " << A*iA;

  COUT << "|A(0)| = " << A.Column(0).Norm() << ENDL;
  COUT << "|A(1)| = " << A.Column(1).Norm() << ENDL;
  COUT << "|A(2)| = " << A.Column(2).Norm() << ENDL;

  Vector yuv(100.0,100.0,100.0);
  Vector yuv2(200.0,200.0,200.0);

  COUT << "A*yuv = " << iA*yuv;

  COUT << "A*yuv2 = " << iA*yuv2;

  /*
  Vector x(3.9,0.0,0.0);
  Vector y(1.0,1.548,1.13);
  Vector a(0.4,1.0,0.0);
  Vector b = a;
  b += x;

  Vector f1 = y.Cross(x);

  double f2 = a*f1;

  Vector xj(3);Vector yj(3);

  if (ABS(f1*f1) < MACHEPS)
    {
      cout << "Fall 1" << endl;
    }
  else
  if (ABS(f2) < MACHEPS)
    {
      cout << "Fall 2" << endl;

      Vector na = a;
      na *= 1/sqrt(a*a);
      Vector nb = b;
      nb *= 1/sqrt(b*b);
      Vector ny = y;
      ny *= 1/sqrt(y*y);
      
      Vector nab = na;
      nab -= nb;
      double dab = nab*nab;
      
      Vector nay = na;
      nay -= ny;
      double day = nay*nay;
      
      Vector nby = nb;
      nby -= ny;
      double dby = nby*nby;
      
      Vector ma = a;
      //  ma *= -1;
      Vector ca = ma.Cross(y);
      Vector mb = b;
      // mb *= -1;
      Vector cb = mb.Cross(y);
      
      double da = sqrt(ca*ca) / sqrt(y*y);
      double db = sqrt(cb*cb) / sqrt(y*y);
      
      cout << "day = " << day << " dby = " << dby << " dab = " << dab << endl;
      
      double dist = MIN(da,db);
      if ((day < dab) && (dby < dab))
	{
	  cout << "Schnitt" << endl;
	  dist = 0;
	}
      else
	{
	  cout << "Rand" << endl;
	}
      
      cout << "da = " << da << " db = " << db << endl;
      
      cout << "Dist = " << dist << endl;
      
      Vector n = a.Cross(y).Cross(y);
      
     
      if ((dist != da) && (dist != db))
	{
	  cout << "Loese:" << endl;
	  Matrix S(2,2);
	  S(0,0) = y[0]; S(0,1) = x[0];
	  S(1,0) = y[1]; S(1,1) = x[1];
	  S.Column(1) *= -1;
	  //      S.Transpose();
	  Vector a1(2);
	  a1[0] = a[0];
	  a1[1] = a[1];
	  
	  Vector l = S.Solve(a1);
	  REQUIRE((l[1] <= 1.0) && (l[1] >= 0));
	  REQUIRE(l[0]*y[2]-l[1]*x[2] == a[2]);
	  Vector z = S*l;
	  cout << "z = " << z << " a1 = " << a1 << endl;
	  REQUIRE(ABS(z[0] - a1[0]) < MACHEPS);
	  REQUIRE(ABS(z[1] - a1[1]) < MACHEPS);
	  cout << "lambda (y,x) = " << l << endl;
	  xj = a;
	  Vector tmp = x;
	  tmp *= l[1];
	  xj += tmp;
	  
	  yj = y;
	  yj *= l[0];
	}
      else
	{
	  if (dist == da)
	    xj = a;
	  else
	    xj = b;
	  
	  Vector n1 = n;
	  n1 *= dist/sqrt(n*n);
	  yj = xj;
	  yj += n1;
	}
    }
  else
    {
      double dist = 0;
      cout << "Fall 3" << endl;
      Vector n = y.Cross(x);

      double ntwo = n*n;
	
      Vector ap = a;
      Vector n1 = n;
      double an = a*n;
      n1 *= an/ntwo;
      ap -= n1;

      Vector bp = b;
      Vector n2 = n;
      double bn = b*n;
      n2 *= bn/ntwo;
      bp -= n2;

      const double y_norm = sqrt(y*y);
      cout << "|ap-H|= " << sqrt((ap*n)*(ap*n))/sqrt(ntwo) << endl;
      cout << "|bp-H|= " << sqrt((bp*n)*(bp*n))/sqrt(ntwo) << endl;

      Vector na = ap;
      na *= (1/sqrt(ap*ap));
      Vector nb = bp;
      nb *= (1/sqrt(bp*bp));
      Vector ny = y;
      ny *= (1/y_norm);

      Vector nab = na;
      nab -= nb;
      double dab = sqrt(nab*nab);
      Vector nay = na;
      nay -= ny;
      double day = sqrt(nay*nay);
      Vector nby = nb;
      nby -= ny;
      double dby = sqrt(nby*nby);

      double denom = a*n;
      double c_norm = sqrt(n*n);
      if ((day < dab) && (dby < dab))
	{
	  double dc = ABS(denom)/c_norm;
	  dist = dc;
	  
	  cout << "Dist =" << dist << endl;

	  cout << "Schnitt" << endl;

	  Matrix S(2,2);
	  S(0,0) = y[0]; S(0,1) = x[0];
	  S(1,0) = y[1]; S(1,1) = x[1];
	  S.Column(1) *= -1;
	  Vector a1(2);
	  a1[0] = ap[0];
	  a1[1] = ap[1];
	    
	  Vector l = S.Solve(a1);

	  cout << "Lambda (y,x) = " << l << endl;
	  REQUIRE((l[1] <= 1.0) && (l[1] >= 0));
	  cout << "[3]: " << l[0]*y[2]-l[1]*x[2]- ap[2] << endl;
	  REQUIRE(ABS(l[0]*y[2]-l[1]*x[2]- ap[2]) < EPSILON);
	    
	  xj = a;
	  Vector tmp = x;
	  tmp *= l[1];
	  xj += tmp;
	  
	  yj = y;
	  yj *= l[0];
	}
      else
	{
	  cout << "Rand ";

	  Vector a_cross_y = a.Cross(y);
	  Vector b_cross_y = b.Cross(y);
	  
	  double da = sqrt(a_cross_y*a_cross_y)/y_norm;
	  double db = sqrt(b_cross_y*b_cross_y) /y_norm;
	  double dxy = 0;
	  if (da < db)
	    {
	      cout << " A" << endl;
	      
	      dxy = ap*y;
	      
	      dist = da;
	      xj = a;
	      yj = ap;
	      cout << "Dist =" << dist << endl; 
	    }
	  else
	    {
	      cout << " B" << endl;
	      
	      dxy = bp*y;
	      dist = db;
	      cout << "Dist =" << dist << endl;
	      xj = b;
	      yj = bp;
	    }

	  cout << "dxy = " << dxy << endl;
	  yj = y;
	  yj *= dxy/(y*y);

	  double dy = sqrt(yj.Cross(y)*yj.Cross(y))/y_norm;
	  cout << "|yj-y| = " << dy << endl;
	  cout << "|H-yj| = " << sqrt((yj*n)*(yj*n))/sqrt(n*n) << endl;
	}
    }

  cout << "xj = " << xj << "yj = " << yj << endl;
      
  Vector diff = xj;
  diff -= yj;

  double cdist = sqrt(diff*diff);

  cout << "diff = " << cdist << endl;
  cout << "|y cross yj| = " << y.Cross(yj)*y.Cross(yj) << endl ;

  Vector yn = y;
  yn *= 1/sqrt(y*y);

  double depth = (ABS(yn[0]) > MACHEPS ? yj[0]/yn[0] : yj[1]/yn[1]);
  cout << "Depth = " << depth << endl;

  Vector xjs = xj;
  xjs -= a;
  cout << "|x cross xjs| = " << x.Cross(xjs)*x.Cross(xjs) << endl ;

  cout << "|x| = " << sqrt(x*x) << " |xj| = " << sqrt(xjs*xjs) << endl;
  */

    
  return 0;
}
