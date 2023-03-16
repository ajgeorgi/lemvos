
// #include "Projekt.h"
#include "hmatrix.h"

// #include <string.h>

#include <math.h>
/*
#ifndef CHECK
#define CHKDIM(_x) if (!(_x)) { theError = ErrorDim; return }
#else
#define CHKDIM(_x) REQUIRE(_x)
#end
*/

#ifdef CHECK
#define MATRIXIDENTIFYER "I am a Matrix"
#define VECTORIDENTIFYER "I am a Vector"
#endif

// #define SWAP(_n,_m) { unsigned _x = (_m); _m = _n; _n = _x; }
#define	sgn(x)	( (x) >= 0 ? 1 : -1 )
// #define MIN(_a,_b) ((_a) < (_b) ? (_a) : (_b))

Vector &Vector::operator -= (const Vector &v)
{
  if (theData == NULL)
    CreateData(v);

  REQUIRE(_GetDim() == v._GetDim());

  CopyData();

  for ( unsigned i = 0;i < _GetDim();i++)
    (*this)[i] -= v[i];

  return *this;
}

Vector &Vector::operator += (const Vector &v)
{
  if (theData == NULL)
    CreateData(v);

  REQUIRE(_GetDim() == v._GetDim());
  REQUIRE(_GetDim() > 0);

  CopyData();

  for ( unsigned i = 0;i < _GetDim();i++)
    (*this)[i] += v[i];

  return *this;
}

Vector &Vector::operator *= (double val)
{
  REQUIRE(theData);
  REQUIRE(_GetDim() > 0);

  CopyData();

  for (unsigned i = 0;i < _GetDim();i++)
    (*this)[i] *= val;

  return *this;
}

Vector Vector::Cross(const Vector &v) const
{
  REQUIRE((_GetDim() == 3) && (v._GetDim() == 3));
  
  Vector x(MAX(GetDimension(),v.GetDimension()));

  if (IsHomogen() || v.IsHomogen())
    {
      x.Homogen();
      x[x.GetDimension()-1] = 1.0;
    }

  x[0] = (*this)[1]*v[2] - (*this)[2]*v[1];
  x[1] = (*this)[2]*v[0] - (*this)[0]*v[2];
  x[2] = (*this)[0]*v[1] - (*this)[1]*v[0];

  return x;
}

const Vector &Vector::operator = (const Vector &v2)
{ 

  if (v2.theData == NULL)
  {
    RemoveData();
    return (*this);
  }

  if (this == &v2)
    return *this;

  if (!theOwnData && theData)
  {
    REQUIRE(GetDimension() >= v2.GetDimension());
    REQUIRE(GetDimension() > 0);
    REQUIRE(!IsHomogen());

    for (unsigned i = 0;i < v2.GetDimension();i++)
       (*this)[i] = v2[i];
  }
  else
  {
    REQUIRE(v2.theData);
    RemoveData();
    AddData(v2);
  }

  REQUIRE(theData);

  return *this;
}

const Vector &Vector::operator = (const double *data)
{ 
  REQUIRE(data);

  const unsigned dim = GetDimension();

  REQUIRE(dim > 0);
   
  if (theData == NULL)
    {
      theData = new _VectorData(dim);
      theOwnData = true;
    }
  else
    {
      CopyData();
    }
    
  REQUIRE(theData);
  REQUIRE(GetDimension() > 0);

  memcpy(theData->theData,data,sizeof(double)*dim);
  theData->isZero = false;

  return *this;
}

Matrix Vector::operator & (const Vector &v) const
{
  REQUIRE(theData && v.theData);

  Matrix M(GetDimension(),v.GetDimension());

  for (unsigned i = 0;i < _GetDim();i++)
    {
       for (unsigned j = 0;j < v._GetDim();j++)
	 {
	   M(i,j) = (*this)[i]*v[j];
	 }
    }

  if (v.IsHomogen() || IsHomogen())
    {
      M(GetDimension()-1,v.GetDimension()-1) = 1.0;
    }

  return M;
}

double Vector::operator * (const Vector &v) const
{
  if ((theData == NULL) || (v.theData == NULL))
    return 0.0;

  REQUIRE(_GetDim() == v._GetDim());

   double sum = 0.0;

  for( unsigned i = 0;i < _GetDim();i++)
    sum += (*this)[i]*v[i];

  return sum;
}

Matrix::Matrix(unsigned  n,unsigned m)
  : theError(NoError),
    isTransponate(false),
    theData(new _MatrixData(n,m)),    
    theQRFactor(NULL),
    theQRDiag(NULL)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(MATRIXIDENTIFYER)
#endif
{
}

Matrix::Matrix()
  : theError(NoError),   
    isTransponate(false),    
    theData(NULL),
    theQRFactor(NULL),
    theQRDiag(NULL)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(MATRIXIDENTIFYER)
#endif
{
}

Matrix::Matrix(const Matrix& matrix)
  : theError(NoError),
    isTransponate(false),    
    theData(NULL),
    theQRFactor(NULL),
    theQRDiag(NULL)   
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(MATRIXIDENTIFYER)
#endif
{
  AddData(matrix);
}

Matrix::~Matrix()
{
  RemoveData();
}

void Matrix::Transpose()
{
  REQUIRE(theData);
  SWAP(theData->dim1,theData->dim2);
  isTransponate = !isTransponate;
}
 
const Matrix& Matrix::operator = (const Matrix &matrix)
{
  if (this == &matrix)
    return *this;

  RemoveData();
  AddData(matrix);

  return *this;
}

const Matrix&  Matrix::operator += (const Matrix &matrix)
{
  if (theData == NULL)
      CreateData(matrix);
  
  CopyData();

  for ( unsigned i = 0;i < theData->dim1;i++)
  for ( unsigned j = 0;j < theData->dim2;j++)
  {
    (*this)(i,j) += matrix(i,j);
  }

  return *this;
}

const Matrix&  Matrix::operator *= (double val)
{
  CopyData();

  for ( unsigned i = 0;i < theData->dim1;i++)
  for ( unsigned j = 0;j < theData->dim2;j++)
  {
    (*this)(i,j) *= val;
  }

  return *this;
}

const Matrix&  Matrix::operator -= (const Matrix &matrix)
{
  if (theData == NULL)
    CreateData(matrix);

  CopyData();

  for ( unsigned i = 0;i < theData->dim1;i++)
  for ( unsigned j = 0;j < theData->dim2;j++)
  {
    (*this)(i,j) -= matrix(i,j);
  }

  return *this;
}

void Matrix::CreateData(const Matrix& m)
{
  RemoveData();
  
  theData = new _MatrixData(m.GetDim1(),m.GetDim2());

  //   isHomogen = m.isHomogen;
}

void Matrix::RemoveData()
{
  if (theData)
    {
      if (theData->theRefCount <= 1)
	{
	  if (theQRFactor)
	    delete theQRFactor;
	  if (theQRDiag)
	    delete theQRDiag;
	  delete theData;
	}
      else
	theData->theRefCount--;

      theData = NULL;
    }

  theQRFactor = NULL;
  theQRDiag = NULL;
  
  isTransponate = false;
}

void Matrix::AddData(const Matrix &matrix)
{
  RemoveData();

  if (matrix.theData == NULL)
    return;

  REQUIRE(matrix.theData);
  theData = matrix.theData;
  theData->theRefCount++;
  isTransponate = matrix.isTransponate;
  
  theQRFactor = matrix.theQRFactor;
  theQRDiag = matrix.theQRDiag;

  theError = matrix.theError;
}

Vector Matrix::operator * (const Vector &x) const
{
  REQUIRE(theData);
  REQUIRE(x.GetDimension() ==  theData->dim2);

  Vector b(GetDim1());
  if (x.IsHomogen())
    {
      b.Homogen();
      // b[b.GetDimension()-1] = 0.0;
    }
 
  REQUIRE(b.GetDimension() == GetDim1());
  REQUIRE(x.GetDimension() == GetDim2());
  
  for ( unsigned i = 0;i < theData->dim1;i++)
    for ( unsigned j = 0;j < theData->dim2;j++)
      {
	b[i] += (*this)(i,j)*x[j];
      }
  
  return b;
}

void Matrix::CopyData()
{
  if (theData == NULL) // Nothing to copy
    return;

  if (theData->theRefCount == 1) // I'am the only owner of the data.
    { // No need to copy. Just kill the QR-Factorization.
      if (theQRFactor)
	delete theQRFactor;
      if (theQRDiag)
	delete theQRDiag;
      
      theQRFactor = NULL;
      theQRDiag = NULL;

      return;
    }

  // QR-Factorization invalid because the matrix will change.
  theQRFactor = NULL;
  theQRDiag = NULL;

  //  cout << "Copy Data" << endl;

  // Create new own data
  _MatrixData *data = new _MatrixData(theData->dim1,theData->dim2);
  
  REQUIRE(data);
  REQUIRE(data->theData);
  REQUIRE(data->theRefCount == 1);

  // Do the real copy
  if (isTransponate)
    { // Transponated matrizes are copied in transponated order
      for ( unsigned i = 0;i < theData->dim1;i++)
	for ( unsigned j = 0;j < theData->dim2;j++)
	  {
	    data->theData[j][i] = theData->theData[i][j];
	  }

      isTransponate = false;
      SWAP(theData->dim1,theData->dim1);
    }
  else
    { // Any other matrizes are just memory copied
      for ( unsigned i = 0;i < GetDim2();i++)
	{
	  memcpy(data->theData[i],theData->theData[i],
		 sizeof(double)*theData->dim1);
	}
    }

  // Remove old data
  RemoveData();

  // Add new own data
  if (theData)
    data->isZero = theData->isZero;
  theData = data;
}


std::ostream &operator << (std::ostream& os,const Matrix& matrix)
{
  os << "Mat(" << matrix.GetDim1() << "," <<  matrix.GetDim2() << "):[  " << std::endl;
  for (unsigned i = 0;i <  matrix.GetDim1();i++)
    {
      for (unsigned j = 0;j <  matrix.GetDim2();j++)
	{
	  os << matrix(i,j) << "  ";
	}
      if (i < matrix.GetDim1()-1)
	os << std::endl;
      else
	os << "]" << std::endl;
    }

  return os;
}

std::ostream &operator << (std::ostream& os,const Vector& v)
{
  if (v.IsHomogen())
    os << "H";
  os << "Vec(" << v.GetDimension()  << "):[  ";
  for (unsigned i = 0;i < v.GetDimension();i++)
    {
      os << v[i] << "  ";
    }
  os << "]" << std::endl;

  return os;
}


Matrix Matrix::operator * (const Matrix &M) const
{
  REQUIRE(M.GetDim1() == GetDim1());
  REQUIRE(M.GetDim2() == GetDim2());

  Matrix N(GetDim1(),GetDim2());
  for ( unsigned j = 0;j < GetDim1();j++)
    {
      N.Column(j) = (*this) * M.Column(j);
    }

  return N;
}


Vector Matrix::_Column(unsigned col) const
{
  REQUIRE(GetDim1() > col);

  Vector c(GetDim1());
  c = (const double *) theData->theData[col];
  REQUIRE(c.theData);

  return c;
}

Vector Matrix::_Row(unsigned row) const
{
  REQUIRE(row < theData->dim2);
  
  Vector c(GetDim2());
  
  for ( unsigned i = 0;i < GetDim2();i++)
    {
      c[i] = (*this)(i,row);
    }

  return c;
}

Vector Matrix::_Column(unsigned col)
{
  REQUIRE(col < theData->dim1);
  
  CopyData();

  Vector c(theData->theData[col],theData->dim2);
  REQUIRE(c.theData);
  REQUIRE(c.IsReference());
  
  return c;
}

Vector Matrix::_Row(unsigned row)
{
  REQUIRE(row < theData->dim1);

  CopyData();

  Vector c(theData->theData,theData->dim2,row);
 
  return c;
}

void Vector::RemoveData()
{
  if (theData)
    {
      if (theData->theRefCount <= 1)
	{
	  if (theOwnData)
	    delete theData->theData;
	  delete theData;	  
	}
      else
	theData->theRefCount--;

      theData = NULL;
    }
}

void Vector::AddData(const Vector &v)
{
  RemoveData();

  if (v.theData == NULL)
    return;
  
  REQUIRE(v.theData);
  REQUIRE(v.theDimension > 0);

  theData = v.theData;
  theData->theRefCount++;
  theOwnData = v.theOwnData;
  theDimension = v.theDimension;
  isHomogen = v.isHomogen;
}

Vector::~Vector()
{
  RemoveData();
}

void Vector::CreateData(const Vector &v)
{
  RemoveData();

  REQUIRE(v.theData);
  REQUIRE(v.theDimension > 0);

  theDimension = v.theDimension;
  isHomogen = v.isHomogen;
  theData = new _VectorData(v.GetDimension());
  theOwnData = true;
}

void Vector::CopyData()
{
  if (theData == NULL)
    return;

  if (!theOwnData)
    {
      theData->theNorm = -1.0;
      return;
    }

  if (theData->theRefCount == 1)
    {
      theData->theNorm = -1.0;
      return;
    }

  REQUIRE(theDimension > 0);

  _VectorData *data = new _VectorData(theDimension);

  REQUIRE(data);

  memcpy(data->theData,theData->theData,
	 sizeof(double)*theDimension);

  RemoveData();

  theData = data;
  theOwnData = true;
}

Vector::Vector(unsigned dim)
  : isHomogen(false),
    theData(new _VectorData(dim)),
    theDimension(dim),
    theOwnData(true)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  REQUIRE(dim > 0);
  REQUIRE(theData);
}

Vector::Vector(double x1,double x2,double x3,double x4,bool homogen)
  : isHomogen(homogen),
    theData(new _VectorData(4)),
    theDimension(4),
    theOwnData(true)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  REQUIRE(theData);

  (*this)[0] = x1;
  (*this)[1] = x2;
  (*this)[2] = x3;
  (*this)[3] = x4;
}

Vector::Vector(double x1,double x2,double x3)
  : isHomogen(false),
    theData(new _VectorData(3)),
    theDimension(3),
    theOwnData(true)        
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  REQUIRE(theData);

  (*this)[0] = x1;
  (*this)[1] = x2;
  (*this)[2] = x3;
}

Vector::Vector(double x1,double x2)
  : isHomogen(false),
    theData(new _VectorData(2)),
    theDimension(2),
    theOwnData(true)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  REQUIRE(theData);
  
  (*this)[0] = x1;
  (*this)[1] = x2;
}


Vector::Vector()
  : isHomogen(false),
    theData(NULL),    
    theDimension(0),
    theOwnData(false)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
}

Vector::Vector(double *data,unsigned d)
  : isHomogen(false),
    theData(new _VectorData(data)),
    theDimension(d),
    theOwnData(false)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
}

Vector::Vector(double **data,unsigned d,int r)
  : isHomogen(false),
    theData(new _VectorData(r,data)),
    theDimension(d),
    theOwnData(false)        
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  REQUIRE(theData);
  REQUIRE(theData->theRowData);
  REQUIRE(theData->theData == NULL);
}

Vector::Vector(const Vector &v)
  : isHomogen(v.isHomogen),
    theData(NULL),
    theDimension(v.GetDimension()),
    theOwnData(false)
#if defined(CHECK) || defined(DEBUG_CHECK)
  ,theIdentifyer(VECTORIDENTIFYER)
#endif
{
  AddData(v);
}

static void Givens(double x,double y,double &c,double &s)
{
	const double norm = sqrt(x*x+y*y);
	if ( norm == 0.0 )
	{	c = 1.0;	s = 0.0;	}	/* identity */
	else
	{	c = x/norm;	s = y/norm;	}
}

Matrix &Matrix::RotCols(Matrix &out,unsigned i,unsigned k,double c,double s)
{
  REQUIRE(out.GetDim1() && out.GetDim2());
  
  REQUIRE((i < out.GetDim2()) &&
	  (k < out.GetDim1()));
  
  for (  unsigned j=0; j<out.GetDim1(); j++ )
    {
      double temp = c*out(j,i) + s*out(j,k);
      out(j,k) =  -s*out(j,i) + c*out(j,k);
      out(j,i) = temp;
    }
  
  return (out);
}

const Matrix& Matrix::operator = (const double * data)
{
  REQUIRE(theData);

  const unsigned  dim1 = GetDim1();
  const unsigned  dim2 = GetDim2();

  RemoveData();

  theData = new _MatrixData(dim1,dim2);

  const double *dataPtr = data;
  //   for ( unsigned i = 0;i < theData->dim1;i++)
  for ( unsigned j = 0;j < theData->dim2;j++)
  for ( unsigned i = 0;i < theData->dim1;i++)
  {
    theData->theData[j][i] = *dataPtr;
    dataPtr++;
  }
  
  // memcpy(theData->theData,data,dim1*dim2*sizeof(double));

  return *this;
}

Vector Matrix::TrieEigen(Vector& a,Vector& b,Matrix& Q) const
{
  REQUIRE(Q.GetDim1() == Q.GetDim2());
  REQUIRE((a.GetDimension() == b.GetDimension() + 1) &&
	  (Q.GetDim1() == a.GetDimension()));
  REQUIRE(Q.GetDim1() == Q.GetDim2());
  REQUIRE(!a.IsHomogen() && !b.IsHomogen());

  int n = (int)a.GetDimension();

  // cout << "a = " << a;
  // cout << "b = " << b;
  int i_min = 0;

  while ( i_min < n )		/* outer while loop */
    {
      /* find i_max to suit;
	 submatrix i_min..i_max should be irreducible */
      int i_max = n-1;
      for (int i = i_min; i < n-1; i++ )
	if ( b[i] == 0.0 )
	  {	i_max = i;	break;	}
      if (i_max <= i_min )
	{	  
	  i_min = i_max + 1;
	  continue;	/* outer while loop */
	}

      /* repeatedly perform QR method until matrix splits */
      bool split = false;
      while ( ! split )		/* inner while loop */
	{
	  /* find Wilkinson shift */
	  double d = (a[i_max-1] - a[i_max])/2;
	  double b_sqr = b[i_max-1]*b[i_max-1];
	  double mu = a[i_max] - b_sqr/(d + sgn(d)*sqrt(d*d+b_sqr));
	  	  
	  /* initial Givens' rotation */
	  double c,s;
	  Givens(a[i_min]-mu,b[i_min],c,s);
	  s = -s;
	  
	  double c2,s2;
	  if ( fabs(c) < M_SQRT2 )
	    {	c2 = c*c;	s2 = 1-c2;	}
	  else
	    {	s2 = s*s;	c2 = 1-s2;	}
	  double cs = c*s;
	  double ak1 = c2*a[i_min]+s2*a[i_min+1]-2*cs*b[i_min];
	  double bk1 = cs*(a[i_min]-a[i_min+1]) +
	    (c2-s2)*b[i_min];
	  double ak2 = s2*a[i_min]+c2*a[i_min+1]+2*cs*b[i_min];
	  double bk2 = ( i_min < i_max-1 ) ? c*b[i_min+1] : 0.0;
	  double z  = ( i_min < i_max-1 ) ? -s*b[i_min+1] : 0.0;
	  a[i_min] = ak1;
	  a[i_min+1] = ak2;
	  b[i_min] = bk1;
	  if ( i_min < i_max-1 )
	    b[i_min+1] = bk2;
	  
	  RotCols(Q,i_min,i_min+1,c,-s);
	
	  // cout << "# z = " << z << endl;
	  // cout << "# a [temp1] = " << a;
	  // cout << "# b [temp1] = " << b;

	  for (int i = i_min+1; i < i_max; i++ )
	    {
	      /* get Givens' rotation for sub-block -- k == i-1 */
	      Givens(b[i-1],z,c,s);
	      s = -s;
	      /* perform Givens' rotation on sub-block */
	      if ( fabs(c) < M_SQRT2 )
		{	c2 = c*c;	s2 = 1-c2;	}
	      else
		{	s2 = s*s;	c2 = 1-s2;	}
	      double cs = c*s;
	      double bk  = c*b[i-1] - s*z;
	      ak1 = c2*a[i]+s2*a[i+1]-2*cs*b[i];
	      bk1 = cs*(a[i]-a[i+1]) +
		(c2-s2)*b[i];
	      ak2 = s2*a[i]+c2*a[i+1]+2*cs*b[i];
	      bk2 = ( i+1 < i_max ) ? c*b[i+1] : 0.0;
	      z  = ( i+1 < i_max ) ? -s*b[i+1] : 0.0;
	      a[i] = ak1;	a[i+1] = ak2;
	      b[i] = bk1;
	      if ( i < i_max-1 )
		b[i+1] = bk2;
	      if ( i > i_min )
		b[i-1] = bk;
	      
	      RotCols(Q,i,i+1,c,-s);
	      // cout << "# a [temp2] = " << a;
	      // cout << "# b [temp2] = " << b;
	    }

	  /* test to see if matrix should be split */
	  for (  int i = i_min; i < i_max; i++ )
	    if ( fabs(b[i]) < MACHEPS*
		 (fabs(a[i])+fabs(a[i+1])) )
	      {   b[i] = 0.0;	split = true;	}

	  // cout << "# a  = " << a;
	  // cout << "# b  = " << b;

	  // cout << "Q = " << Q;
	}
    }
  
  return a;
}

static Vector &hhtrvec(const Vector &hh,double beta,unsigned i0,const Vector &in,Vector &out)
{
  REQUIRE(in.GetDimension() == hh.GetDimension());
  REQUIRE(i0 <= in.GetDimension());
  REQUIRE(!hh.IsHomogen());
  REQUIRE(!in.IsHomogen());

  double prod = hh.Cut(i0)*in.Cut(i0);
  double scale = beta*prod;

  /*
  cout << "prod = " << prod << endl;
  cout << "beta = " << beta << endl;
  cout << "hh = " << hh;
  cout << "scale = " << scale << endl;
  */

  for (  unsigned i=i0; i<in.GetDimension(); i++ )
      out[i] = in[i] - scale*hh[i];
  
  return (out);
}


static Matrix &makeHQ(const Matrix &H,const Vector &diag,const Vector& beta, Matrix &Qout)
{
  unsigned limit = H.GetDim1() - 1;
  REQUIRE(diag.GetDimension() >= limit);
  REQUIRE(beta.GetDimension() >= limit);
  REQUIRE(H.GetDim1() == H.GetDim2());
  REQUIRE(!beta.IsHomogen());
  REQUIRE(!diag.IsHomogen());

  Qout = H;
  //  Qout.Zero();

  Vector tmp1(H.GetDim1());
  
  for (int i = 0; i < (int) H.GetDim1(); i++ )
    {
      tmp1.Unit(i);
      //      cout << "1.tmp1= " << tmp1;
      /* apply H/h transforms in reverse order */
      // Vector tmp2;
      for (  int j = limit-1; j >= 0; j-- )
	{
	  Vector tmp2 = H.Column(j);
	  REQUIRE(!tmp2.IsReference());
	  //  cout << "tmp2 = " << tmp2;
	  
	  tmp2[j+1] = diag[j];
	  hhtrvec(tmp2,beta[j],j+1,tmp1,tmp1);
	  // cout << "tmp1 = " << tmp1;
	}
      
      /* insert into Qout */
      Qout.Column(i) = tmp1;
    }
  
  return (Qout);
}


Vector Matrix::SymEigen(Matrix& Q) const
{
  Vector out(GetDim1());
  
  REQUIRE(GetDim1() == GetDim2());
  
  Matrix tmp  = (*this);
  Vector b(GetDim1() - 1);
  Vector diag(GetDim1());
  Vector beta(GetDim1());
  
  Hfactor(tmp,diag,beta);
  
  makeHQ(tmp,diag,beta,Q);

  for (int i = 0; i < (int)GetDim1() - 1; i++ )
    {
      out[i] = tmp(i,i);
      b[i] = tmp(i,i+1);
    }

  int i = GetDim1()-1;
  out[i] = tmp(i,i);

  // cout << "Q X = " << Q;

  TrieEigen(out,b,Q);
  
#ifdef CHECK
  //  cout << "lambda = " << out;
  //  cout << "Q = " << Q;
  for (int i = 0;i < (int)GetDim2();i++)
    {
      if (out[i] > MACHEPS*100)
	{
	  Vector l = (*this)*Q.Column(i);
	  l *= 1.0/out[i];
	  l -= Q.Column(i);
	  if (l.Norm() > EPSILON)
	    {
	      theError = ErrorEV;
	      /*
	      cout << "iter = " << i << endl;
	      cout << "err = " << l.Norm() << endl;
	      cout << "err % = " << (Q.Column(i).Norm() / l.Norm()) << endl;
	      cout << "lambda = " << out;
	      cout << "Q = " << Q;
	      cout << "A*Q.C(" << i << ")= " << (*this)*Q.Column(i) << endl;
	      cout << "A = " << (*this); 
	      REQUIRE(false);
	      */
	    }
	}
    }
#endif  

  return out;
}


Matrix &hhtrrows(Matrix &M,
			 unsigned i0,unsigned j0,
			 const Vector& hh,double beta)
{
  REQUIRE(M.GetDim2() == hh.GetDimension());
  REQUIRE((i0 <= M.GetDim1()) && (j0 <= M.GetDim2()));
  REQUIRE(!hh.IsHomogen());

  if ( beta == 0.0 )	return (M);
  
  /* for each row ... */
  for (int  i = i0; i < (int)M.GetDim1(); i++ )
    {	/* compute inner product */
         
      double ip = 0.0;
      for ( unsigned j = j0; j < M.GetDim2(); j++ )
	ip += M(i,j)*hh[j];
      
      double scale = beta*ip;
      if ( scale == 0.0 )
	continue;
      
      for ( unsigned  j = j0; j < M.GetDim2(); j++ )
	M(i,j) -= scale*hh[j];
    }
  
  return (M);
}


static Matrix &hhtrcols(Matrix &M,
			 unsigned i0,unsigned j0,
			 const Vector &hh, double beta)
{
  REQUIRE(M.GetDim1() == hh.GetDimension());
  REQUIRE((i0 <= M.GetDim1()) &&
	  (j0 <= M.GetDim2()));
  REQUIRE(!hh.IsHomogen());
   
  if ( beta == 0.0 )	return (M);
  
  Vector w(M.GetDim2());
  
  for (int i = i0; i < (int)M.GetDim1(); i++ )
    if ( hh[i] != 0.0 )
      {
	for ( unsigned j = j0; j < M.GetDim2(); j++ )
	  w[j] += M(i,j)*hh[i];
      }
  
  for (int i = i0; i < (int)M.GetDim1(); i++ )
    if ( hh[i] != 0.0 )
      {
	for ( unsigned  j = j0; j < M.GetDim2(); j++ )
	  M(i,j) -= beta*hh[i]*w[j];
      }
  
  return (M);
}

static Vector &hhvec(const Vector &vec,
		      unsigned i0,double &beta,
		      Vector &out,double &newval)
{
  //  REQUIRE(vec.theData);
  
  out = vec;
  REQUIRE(!vec.IsHomogen());
  REQUIRE(!out.IsHomogen());

  //  REQUIRE(out.theData);
  
  Vector sub = out.Cut(i0);
  REQUIRE(sub.GetDimension() == out.GetDimension() - i0);
  double norm = sub.Norm();

  if ( norm <= 0.0 )
    {
      beta = 0.0;
      return (out);
    }

  beta = 1.0/(norm * (norm+fabs(out[i0])));
  if ( out[i0] > 0.0 )
    newval = -norm;
  else
    newval = norm;

  out[i0] -= newval;

  return out;
}

Matrix &Matrix::Hfactor(Matrix &A,Vector &diag,Vector &beta)
{
  REQUIRE((diag.GetDimension() >= A.GetDim1() - 1) &&
	  (beta.GetDimension() >= A.GetDim1() - 1) );
  REQUIRE(!diag.IsHomogen());
  REQUIRE(!beta.IsHomogen());
  REQUIRE(A.GetDim1() == A.GetDim2());
  
  unsigned limit = A.GetDim1() - 1;
  Vector tmp1;

  for (unsigned  k = 0; k < limit; k++ )
    {
      tmp1 = ((const Matrix*)&A)->Column(k);

      REQUIRE(tmp1.theData);
      REQUIRE(!tmp1.IsReference());

      hhvec(tmp1,k+1,beta[k],tmp1,A(k+1,k));
      
      diag[k] = tmp1[k+1];
      
      hhtrcols(A,k+1,k+1,tmp1,beta[k]);
      hhtrrows(A,0  ,k+1,tmp1,beta[k]);
    }
  
  return (A);
}


Vector Vector::Cut(unsigned l) const
{
  REQUIRE(l < GetDimension());
  REQUIRE(!IsHomogen());
  REQUIRE(theData);
  
  int dim = GetDimension() - l;
  REQUIRE(dim >= 0);
  Vector v(dim);

  REQUIRE(v.theData);
  REQUIRE(v.theDimension == (unsigned)dim);
  REQUIRE(v.theOwnData);
  
  if (theData->theRowData == NULL)
  {
    memmove(v.theData->theData,&theData->theData[l],sizeof(double)*dim);
  }
  else
    {
	  for(unsigned i = 0;i < v.GetDimension();i++)
      {
	    v[i] = (*this)[i+l];
      }
    }
  REQUIRE(!v.IsReference());

  return v;
}

Vector Vector::Cut(unsigned l)
{
  REQUIRE(l < GetDimension());
  REQUIRE(!IsHomogen());
  REQUIRE(theData);
  Vector v;
  REQUIRE(v.theData == NULL);
  if (theData->theRowData == NULL)
    v.theData = new _VectorData((double *) &theData->theData[l]);
  else
    {
      int row = theData->theRow;
      if (row < 0)
	row = -l-1;
      v.theData = new _VectorData(row,&theData->theRowData[l]);
    }
  v.theDimension = GetDimension() - l;
  REQUIRE(v.IsReference());

  return v;
}

/*
const Vector Vector::Cut(unsigned l) const
{
  REQUIRE(l < GetDimension());

  REQUIRE(theData);
  Vector v;
  REQUIRE(theData->theRowDist == 0);
  v.theData = new _VectorData((double *) &theData->theData[l]);
  v.theDimension = GetDimension() - l;

  return v;
}
*/

void Matrix::Zero()
{
  if (theData == NULL)
    return;

  if (theData->isZero)
    return;

  CopyData();

  REQUIRE(GetDim1() > 0);
  REQUIRE(GetDim2() > 0);

  unsigned dim = 1;
  if (!theData->isDiag)
    dim =  GetDim2();

  for (unsigned i = 0;i < dim;i++)
    memset(theData->theData[i],0,GetDim1()*sizeof(double));

  isTransponate = false;
  
  theData->isZero = true;
}

void Vector::Zero()
{
  if (theData == NULL)
    return;

  if (theData->isZero)
    return;

  CopyData();

  REQUIRE(GetDimension() > 0);
  
  if (theData->theRowData)
    {
      for (unsigned i = 0;i < _GetDim();i++)
	(*this)[i] = 0.0;
    }
  else
    memset(theData->theData,0,_GetDim()*sizeof(double));

  theData->isZero = true;
  if (isHomogen)
    (*this)[GetDimension()-1] = 1.0;
}
/*
std::istream &operator >> (std::istream& is,Matrix& mat)
{
  NOTIMP;
  return is;
}

std::istream &operator >> (std::istream& is,Vector& vec)
{
  NOTIMP;
  return is;
}
*/
void Matrix::Fill(double val)
{
  if (theData == NULL)
    return;

  CopyData();

  unsigned dim = 1;
  if (!theData->isDiag)
    dim =  GetDim2();

  theData->isZero = false;
  for ( unsigned i = 0;i < GetDim1();i++)
    for ( unsigned j = 0;j < dim;j++)
      theData->theData[i][j] = val;
}

void Matrix::Identity()
{
  REQUIRE(theData);

  CopyData();

  Zero();

  unsigned sz = MIN(GetDim1(),GetDim2());

  for ( unsigned i = 0;i < sz;i++)
    (*this)(i,i) = 1.0;
}

void Matrix::RotateDeg(const Vector &degRot,const Vector &trans)
{
  Vector radRot(3);

  radRot[0] = TORAD(degRot[0]);
  radRot[1] = TORAD(degRot[1]);
  radRot[2] = TORAD(degRot[2]);

  Rotate(radRot,trans);
}

void Matrix::Rotate(const Vector &rot,const Vector &trans)
{
  if ((GetDim1() != 4) || (GetDim2() != 4))
    {
      RemoveData();
      REQUIRE(theData == NULL);
      theData = new _MatrixData(4,4);
    }

  REQUIRE(GetDim1() == 4);
  REQUIRE(GetDim2() == 4);

  REQUIRE(rot.GetDimension() >= 3);
  REQUIRE(trans.GetDimension() >= 3);

  CopyData();

  double s1 = sin(rot[0]);
  double s2 = sin(rot[1]);
  double s3 = sin(rot[2]);

  double c1 = cos(rot[0]);
  double c2 = cos(rot[1]);
  double c3 = cos(rot[2]);

  Vector col1 = Column(0);
  REQUIRE(col1.IsReference());
  col1[0] = c2*c3;
  col1[1] = (c1*s3-s1*s2*c3);
  col1[2] = (c1*s2*c3+s1*s3);
  col1[3] = 0.0;

  Vector col2 = Column(1);
  REQUIRE(col2.IsReference());
  col2[0] = -c2*s3;
  col2[1] = (s1*s2*s3+c1*c3);
  col2[2] = (s1*c3-c1*s2*s3);
  col2[3] = 0.0;

  Vector col3 = Column(2);
  REQUIRE(col3.IsReference());
  col3[0] = -s2;
  col3[1] = -s1*c2;
  col3[2] = c1*c2;
  col3[3] = 0.0;

  Vector col4 = Column(3);
  REQUIRE(col4.IsReference());
  col4[0] = trans[0];
  col4[1] = trans[1];
  col4[2] = trans[2];
  col4[3] = 1.0;

#ifdef CHECK
  Vector co1 = Column(0);
  co1.Homogen();
  Vector co2 = Column(1);
  co2.Homogen();
  Vector co3 = Column(2);
  co3.Homogen();
  
  // Row(3).Unit(3);

  REQUIRE(fabs(co1.Norm() -1) < MACHEPS*1000);
  REQUIRE(fabs(co2.Norm() -1) < MACHEPS*1000);
  REQUIRE(fabs(co3.Norm() -1) < MACHEPS*1000);

  REQUIRE(fabs(co1*co2) < MACHEPS*1000);
  REQUIRE(fabs(co2*co3) < MACHEPS*1000);
  //  cout << "|RC(0)| = " << co1.Norm() << endl;
  //  cout << "|RC(1)| = " << co2.Norm() << endl;
  //  cout << "|RC(2)| = " << co3.Norm() << endl;

  // cout << "RC(0)*RC(1) = " << co1*co2 << endl;
  // cout << "RC(1)*RC(2) = " << co2*co3 << endl;
#endif
}

void Vector::Unit(unsigned i)
{
  REQUIRE(theData);
  REQUIRE(i < GetDimension());

  CopyData();

  Zero();

  (*this)[i] = 1.0;
}

void Vector::Fill(double val)
{
  if (theData == NULL)
    return;

  CopyData();

  theData->isZero = false;
  if (theData->theRowData == NULL)
    for ( unsigned i = 0;i < _GetDim();i++)
      theData->theData[i] = val;
  else
    for ( unsigned i = 0;i < _GetDim();i++)
      (*this)[i] = val;

  /*
  if (isHomogen)
    (*this)[GetDimension()-1] = 1.0;
  */
}

Vector Matrix::Diag() const
{
  REQUIRE(theData);
  unsigned sz = MIN(GetDim1(),GetDim2());
  Vector v(sz);
  for ( unsigned i=0;i < sz;i++)
    v[i] = (*this)(i,i);
  return v;
}

Vector Matrix::Diag()
{
  REQUIRE(theData);
  unsigned sz = MIN(GetDim1(),GetDim2());
  Vector v(theData->theData,sz,-1);
  return v;
}

/*
const Matrix& Matrix::operator *= (const Matrix &)
{
  Matrix M(GetDim1(),GetDim2());
  for ( unsigned i = 0;i < GetDim1();i++)
    {
      for ( unsigned j = 0;j < GetDim2();j++)
	{
	  M.Column(i) 
	}
    }
}
*/

Vector Matrix::SVD(Matrix &U,Matrix &V) const
{
  Matrix A(GetDim1(),GetDim2());
  
  for (unsigned i = 0;i < GetDim1();i++)
    for (unsigned j = 0;j < GetDim1();j++)
      {
	A(i,j) = Column(i)*Column(j);
      }

  Vector lambda = A.SymEigen(V);

  if (A.GetLastError() != NoError)
    {
      theError = ErrorEV;
    }

  for (unsigned i = 0;i < lambda.GetDimension();i++)
    {
      double max = -HUGE_VAL;
      unsigned max_idx = 0;
      for (unsigned j = i;j < lambda.GetDimension();j++)
	{
	  if (lambda[j] > max)
	    {
	      max = lambda[j];
	      max_idx = j;
	    }
	}
      double temp = lambda[i];
      lambda[i] = max;
      lambda[max_idx] = temp;
      Vector temp_vec = ((const Matrix*)&V)->Column(i);
      REQUIRE(!temp_vec.IsReference());
      V.Column(i) = V.Column(max_idx);
      V.Column(max_idx) = temp_vec;
    }
  
  U = (*this)*V;
  
  for ( unsigned i = 0;i < U.GetDim2();i++)
    {
      if (fabs(lambda[i]) > MACHEPS)
	{
	  double sigma = sqrt(fabs(lambda[i]));
	  lambda[i] = sigma;
	  U.Column(i) *= 1.0/sigma ;
	}
      else
	{
	  // U.Column(i).Zero();
	  lambda[i] = 0.0;
	  // U.Column(i).Unit(i);
	}
    }
  
  return lambda;
}

void Matrix::SVD(Matrix &U,Matrix &S,Matrix &V) const
{
  Vector sigma = SVD(U,V);

  S.Zero();

  S.Diag() = sigma;

  /*
  unsigned sz = MIN(S.GetDim1(),GetDim2());
  for (unsigned i= 0;i < sz;i++)
    S(i,i) = sigma[i];
  */
}

void Matrix::Usolve(const Matrix &mat,const Vector &b,Vector &out,double diag) const
{
  unsigned int dim = MIN(mat.GetDim1(),mat.GetDim2());
  REQUIRE(b.GetDimension() == dim);
  REQUIRE(out.GetDimension() == dim);
  REQUIRE(!b.IsHomogen());

  theError = NoError;

  const double tiny = 10.0/HUGE_VAL;

  int i;
  for (i=dim-1; i>=0; i-- )
    if ( b[i] != 0.0 )
      break;
    else
      out[i] = 0.0;
  int i_lim = i;
  
  double sum;

  for (    ; i>=0; i-- )
    {
      sum = b[i];
      
      for (int j = i+1;j <= i_lim;j++)
	{
	  sum -= mat(i,j)*out[j];
	}
      
      if ( diag==0.0 )
	{
	  if ( fabs(mat(i,i)) <= tiny*fabs(sum) )
	    {
	      theError = ErrorUSolve;
	      std::cerr << "Error: Usolve" << std::endl;
	    }
	  else
	    out[i] = sum/mat(i,i);
	}
      else
	out[i] = sum/diag;
    }
}

static void _Qsolve(const Matrix &QR,const Vector &diag,const Vector &b,Vector &x,Vector &tmp)
{
  REQUIRE(!diag.IsHomogen());
  unsigned limit =  MIN(QR.GetDim1(),QR.GetDim2());
  REQUIRE((diag.GetDimension() >= limit) || (b.GetDimension() == QR.GetDim1()));
  
    x = b;
    REQUIRE(x == b);

    for (unsigned k = 0 ; k < limit ; k++ )
    {
	tmp = ((const Matrix*)&QR)->Column(k);
	REQUIRE(!tmp.IsReference());

	double r_ii = fabs(tmp[k]);
	tmp[k] = diag[k];
	double tmp_val = (r_ii*fabs(diag[k]));
	double beta = ( tmp_val == 0.0 ) ? 0.0 : 1.0/tmp_val;
	/* hhtrvec(tmp,beta->ve[k],k,x,x); */
	hhtrvec(tmp,beta,k,x,x);
    }
}

static void QRfactor(Matrix &A,Vector &diag)
{
  unsigned  limit =  MIN(A.GetDim1(),A.GetDim2());
  REQUIRE(diag.GetDimension() >= limit);
  
  double beta;
  for (unsigned k=0; k<limit; k++ )
    {
      /* get H/holder vector for the k-th column */
      Vector tmp1 = ((const Matrix *)&A)->Column(k);
      REQUIRE(!tmp1.IsReference());

      hhvec(tmp1,k,beta,tmp1,A(k,k));
      diag[k] = tmp1[k];
      // cout << "diag[" << k << "]= " << tmp1[k] << endl;
      /* apply H/holder vector to remaining columns */
      hhtrcols(A,k,k+1,tmp1,beta);
    }

  REQUIRE(!diag.IsHomogen());
}

Vector Matrix::Solve(const Vector &b) const
{
  REQUIRE(GetDim1() == GetDim2());
  REQUIRE(GetDim1() == b.GetDimension());
  REQUIRE(!b.IsHomogen());

  theError = NoError;

  Vector x(GetDim2());
  unsigned  limit = MIN(GetDim1(),GetDim2());
  
  if (theQRFactor == NULL)
    {
      REQUIRE(theQRDiag == NULL);

      theQRFactor = new Matrix(*this);
      theQRDiag = new Vector(limit);

      QRfactor(*theQRFactor,*theQRDiag);
    }
#ifdef CHECK
  else
    {
      REQUIRE(theQRDiag);

      Matrix qrf(*this);
      Vector diag(limit);
      QRfactor(qrf,diag);

      REQUIRE(qrf == *theQRFactor);
      REQUIRE(diag == *theQRDiag);
    }
#endif

  REQUIRE(theQRFactor && theQRDiag);

  Vector tmp(limit);
  _Qsolve(*theQRFactor,*theQRDiag,b,x,tmp);
  Usolve(*theQRFactor,x,x,0.0);

  /*
  cout << "QR = " << *theQRFactor;
  cout << "QRd = " << *theQRDiag;
  cout << "b = " << b;
  cout << "Ax = " << (*this)*x;
  */

  // REQUIRE(((*this)*x) == b);

  return x;
}

double Vector::Norm() const
{
  if (theData == NULL)
    return 0.0;

  REQUIRE(theData);

  if (theData->theNorm < 0.0)
    {
      theData->theNorm = sqrt((*this)*(*this));
    }
#ifdef CHECK
  else
    {
      REQUIRE(theData->theNorm == sqrt((*this)*(*this)));
    }
#endif

  return theData->theNorm;
}

void Vector::Normalize()
{
  double norm = Norm();
  if (norm > MACHEPS*100)
    (*this) *= 1/norm;
  else
    Zero();
}

bool Matrix::operator == (const Matrix &m) const
{
  if (&m == this)
  return true;
  if ((m.theData == theData) && (isTransponate == m.isTransponate))
  return true;

  if (((theData == NULL) && (m.theData != NULL)) ||
      ((theData != NULL) && (m.theData == NULL)))
      return false;
  
  if (theData->isZero && m.theData->isZero)
  return true;

  if ((GetDim1() != m.GetDim1()) ||
      (GetDim2() != m.GetDim2()))
  return false;

  for (unsigned i = 0;i < GetDim1();i++)
  for (unsigned j = 0;j < GetDim2();j++)
  
  {
    double diff = (*this)(i,j) - m(i,j);
    if (fabs(diff) > MACHEPS*1000)
    return false;
  }
  return true;
}

bool Vector::operator == (const Vector &v) const
{
  if (&v == this)
  return true;
  if ((v.theData == theData) && (theDimension == v.theDimension))
  return true;

  if (((theData == NULL) && (v.theData != NULL)) ||
      ((theData != NULL) && (v.theData == NULL)))
  return false;
  
  if (theData->isZero && v.theData->isZero)
  return true;

  if (theDimension != v.GetDimension())
  return false;

  for (unsigned i = 0;i < theDimension;i++)
  {
    double diff = (*this)[i] - v[i];

    if (fabs(diff) > MACHEPS*1000)
    return false;
  }

  return true;
}


#ifdef CHECK
bool Vector::ok() const
{
  if (this == NULL)
    return false;

  if (strcmp(VECTORIDENTIFYER,theIdentifyer) != 0)
    return false;

  if (fabs(GetDimension() - _GetDim()) >1)
    return false;

  if (Norm() != sqrt((*this)*(*this)))
    return false;

  if (theData)
    {
      if (theData->theRefCount == 0)
	return false;
      if ((theData->theRow != 0) && ((theData->theRowData == NULL) || (theData->theData != NULL)))
	return false;
      if ((theData->theRow == 0) && (theData->theData == NULL))
	return false;
    }
  else
    {
      COUT << "No data in Vector" << ENDL;
    }

  return true;
}
#endif

/*
VEC	*svd(A,U,V,d)
MAT	*A, *U, *V;
VEC	*d;
{
	static VEC	*f=VNULL;
	int	i, limit;
	MAT	*A_tmp;

	if ( ! A )
		error(E_NULL,"svd");
	if ( ( U && ( U->m != U->n ) ) || ( V && ( V->m != V->n ) ) )
		error(E_SQUARE,"svd");
	if ( ( U && U->m != A->m ) || ( V && V->m != A->n ) )
		error(E_SIZES,"svd");

	A_tmp = m_copy(A,MNULL);
	if ( U != MNULL )
	    m_ident(U);
	if ( V != MNULL )
	    m_ident(V);
	limit = min(A_tmp->m,A_tmp->n);
	d = v_resize(d,limit);
	f = v_resize(f,limit-1);
	MEM_STAT_REG(f,TYPE_VEC);

	bifactor(A_tmp,U,V);
	if ( A_tmp->m >= A_tmp->n )
	    for ( i = 0; i < limit; i++ )
	    {
		d->ve[i] = A_tmp->me[i][i];
		if ( i+1 < limit )
		    f->ve[i] = A_tmp->me[i][i+1];
	    }
	else
	    for ( i = 0; i < limit; i++ )
	    {
		d->ve[i] = A_tmp->me[i][i];
		if ( i+1 < limit )
		    f->ve[i] = A_tmp->me[i+1][i];
	    }


	if ( A_tmp->m >= A_tmp->n )
	    bisvd(d,f,U,V);
	else
	    bisvd(d,f,V,U);

	M_FREE(A_tmp);

	return d;
}

*/

double Matrix::Det() const
{
  REQUIRE(GetDim1() == GetDim2());
  REQUIRE(theData);

  switch(GetDim1())
    {
    case 2:
      return (*this)(0,0)*(*this)(2,2)-(*this)(0,2)*(*this)(1,0);
    case 3:
      return Column(0)*Column(1).Cross(Column(2));
    default:
      theError = ErrorNotImp;
      return 0.0;
    }
}

Matrix Matrix::Invert() const
{
  REQUIRE(theData);

  Matrix X(GetDim2(),GetDim1());
  
  for (unsigned i = 0;i < GetDim2();i++)
    {
      Vector e(GetDim1());
      e.Unit(i);
      X.Column(i) = Solve(e);
    }

  return X;
}
