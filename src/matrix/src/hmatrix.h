#ifndef _MATRIX_H
#define _MATRIX_H

#include <iostream>
#include <string.h>

#include "commen.h"

#define COUT std::cout
#define ENDL  std::endl

#ifdef CHECK

// Stops execution if expressen is not vallid
#define REQUIRE(_x) if (!(_x)) { char _c = 0; COUT << "<REQUIRE>:" << __FILE__ << ":" << __LINE__ << ": *** Requirement failed: " << _STR(_x) << ":</REQUIRE>" << ENDL; std::cin >> _c; }

#else
#define REQUIRE(_x) /**/
#endif

// Not MT safe!

#define MACHEPS 2.22045e-16 
#define EPSILON 0.000001


// #define SWAP(_n,_m) { unsigned _x = (_m); _m = _n; _n = _x; }
// extern "C" void *memset(void *, int , unsigned int ); //size_t );  


class Matrix;

class Vector
{
  friend class Matrix;
 
  // Internal used data for a vector
  struct _VectorData {
    
    _VectorData(unsigned n)
      : theRowData(NULL),
	theRefCount(1),
	theData(new double[n]),
	theRow(0),
	theNorm(-1.0),
	isZero(true)
      { REQUIRE(n > 0);
	memset(theData,0,sizeof(double)*n); };
      
      _VectorData(double *data)
	: theRowData(NULL),
	  theRefCount(1),
	  theData(data),
	  theRow(0),
	  theNorm(-1.0),
	  isZero(false)
      {};
      
      _VectorData(int r,double **data)
	: theRowData(data),
	  theRefCount(1),
	  theData(NULL),
	  theRow(r),
	  theNorm(-1.0),
	  isZero(false)
      {};
      
      ~_VectorData() { };
      
      double **theRowData;
      unsigned theRefCount;
      double  *theData;
      int      theRow;
      double   theNorm;
      bool     isZero;
  };
  
 public:
  
  Vector();
  Vector(unsigned dim);
  Vector(const Vector &v);
  Vector(double,double,double,double,bool homogen = false);
  Vector(double,double,double);
  Vector(double,double);

  ~Vector();

  Vector &Homogen() { /* (*this)[GetDimension()-1] = 1.0;*/ isHomogen = true; return *this; };
  bool IsHomogen() const { return isHomogen; };

  unsigned GetDimension() const { return theDimension; };

  const Vector &operator = (const Vector &v2);
  const Vector &operator = (const double*);
  bool operator == (const Vector &v2) const;

  double &operator [] (unsigned int idx) {
    REQUIRE(theData);
    REQUIRE(idx < theDimension);
    theData->isZero = false;
    if (theData->theRowData)
      { theData->theNorm = -1.0;
	if (theData->theRow >= 0)
	  return theData->theRowData[idx][theData->theRow];
        else
	  return theData->theRowData[idx][idx+ABS(theData->theRow)-1];
      }
    else
      { CopyData();
	return theData->theData[idx];
      }
  };
 
  double operator [] (unsigned int idx) const {
    REQUIRE(theData);
    REQUIRE(idx < theDimension);
    double val = 0.0;
    if (theData->theRowData)
      { if (theData->theRow >= 0)
	  val = theData->theRowData[idx][theData->theRow];
        else
	  val = theData->theRowData[idx][idx+ABS(theData->theRow)-1];
      }
    else
      val = theData->theData[idx];
    return val;
  };

  // inner produkt (scalar produkt)
  double operator * (const Vector&) const;

  // multiply each element by a scalar
  Vector &operator *= (double);

  // vector produkt 
  Matrix operator & (const Vector&) const;

  // subtract an element by an element from a nother vector
  Vector &operator -= (const Vector&);
  Vector &operator += (const Vector&);

  // Get subvector of vector. The last n elements are referenced/copied
  Vector Cut(unsigned) const;
  Vector Cut(unsigned);

  // Crossprodukt of two vectors
  Vector Cross(const Vector &) const;

  // Norm
  double Norm() const;
  // Normalize vector
  void Normalize();

  // Zero vector
  void Zero();
  // Create n'th unit vector
  void Unit(unsigned);
  // Fill vector with one value
  void Fill(double);

  // Is vector a reference. D.e. the vector shares its data with a nother Vector/Matrix
  bool IsReference() const { return !theOwnData; };

#if defined(CHECK) || defined(DEBUG_CHECK)
  bool ok() const;
#endif

 private:

  Vector(double *data,unsigned d);
  Vector(double **data,unsigned d,int r);

  unsigned _GetDim() const { return (theData == NULL ? 0 : (isHomogen ? GetDimension() - 1 : GetDimension())); }

 protected:

  bool isHomogen;

  // Memory management
  void RemoveData();
  void AddData(const Vector&);
  void CopyData();
  void CreateData(const Vector &);

  _VectorData *theData;

  unsigned theDimension;
  bool theOwnData;

#if defined(CHECK) || defined(DEBUG_CHECK)
  const char* theIdentifyer;
#endif

};

class Matrix 
{
  friend class Vector;

  // Internal used data for a matrix
  struct _MatrixData {
    
    _MatrixData(unsigned m,unsigned n)
      : theRefCount(1),
	dim1(m),
	dim2(n),
	theData(new double*[n]),
	isDiag(false),
	isZero(true)
    { for ( unsigned j=0;j<n;j++)
      { theData[j] = new double[m];
        memset(theData[j],0,sizeof(double)*m);
      }
    };

    _MatrixData(unsigned m)
      : theRefCount(1),
	dim1(m),
	dim2(m),
	theData(new double*[1]),
	isDiag(true),
	isZero(true)
    {
        theData[0] = new double[m+1];
        memset(theData[0],0,sizeof(double)*(m+1));
    };

    ~_MatrixData() { 
      if (isDiag)
	delete theData[0];
      else
	for ( unsigned j=0;j<dim2;j++)
	  if (theData[j])
	    delete theData[j]; 
      delete[] theData; };
	    
	    unsigned theRefCount;
	    unsigned dim1;
	    unsigned dim2;
	    double **theData;
	    bool isDiag;
	    bool isZero;
  };
  
 public:

  static const int NoError      =   0;
  static const int ErrorUSolve  =  -1;
  static const int ErrorDim     =  -2;
  static const int ErrorZeroDiv =  -3;
  static const int ErrorEV      =  -4;
  static const int ErrorNotImp  =  -5;

  Matrix(const Matrix&);
  Matrix(unsigned,unsigned);
  Matrix();

  virtual ~Matrix();

  int GetLastError() const { return theError; };

  void Transpose();  

  double operator() (unsigned i,unsigned j) const {
    REQUIRE(theData);
    if (isTransponate)
      SWAP(i,j);
    REQUIRE(i < theData->dim1);
    REQUIRE(j < theData->dim2);
    double val;
    if (theData->isDiag)
      {
	val =  (i == j) ? theData->theData[0][i] : theData->theData[0][theData->dim2];
      }
    else
      val = theData->theData[j][i];
    return val;
  };

  double &operator() (unsigned i,unsigned j) {
    REQUIRE(theData);
    if (isTransponate)
      SWAP(i,j);
    REQUIRE(i < theData->dim1);
    REQUIRE(j < theData->dim2);
    CopyData();
    theData->isZero = false;
     if (theData->isDiag)
      {
	  return (i == j) ? theData->theData[0][i] : theData->theData[0][theData->dim2];
      }
     else
       return theData->theData[j][i];
  };

  const Matrix& operator *= (double);
  const Matrix& operator  = (const Matrix &);
  const Matrix& operator  = (const double *);
  const Matrix& operator += (const Matrix &);
  const Matrix& operator -= (const Matrix &);
  // const Matrix& operator *= (const Matrix &);

  bool operator == (const Matrix &v2) const;

  Matrix operator * (const Matrix &) const;
  Vector operator * (const Vector &) const;

  // dimensions of Matrix
  unsigned GetDim1() const { return theData == 0 ? 0 : theData->dim1; };
  unsigned GetDim2() const { return theData == 0 ? 0 : theData->dim2; };

  // Clear Matrix (e.g fill with 0.0)
  void Zero();
  // Fill Matrix with any value
  void Fill(double);
  // Make identity of Matrix
  void Identity();
  // Make homogen rotation matrix with rotation angels in radiant and translations
  void Rotate(const Vector &rot,const Vector &trans);
  // Make homogen rotation matrix with rotation angels in degree and translations
  void RotateDeg(const Vector &rot,const Vector &trans);

  // Create inverse matrix
  Matrix Invert() const;

  // Calculate determinate of matrix (n = 2 and n = 3)
  double Det() const;

  // Get diagonale of matrix
  Vector Diag() const;
  Vector Diag();

  // Get Row and column of matrix
  Vector Column(unsigned col)       { return isTransponate ? _Row(col) : _Column(col); };
  Vector Row(unsigned col)          { return !isTransponate ? _Row(col) : _Column(col); };
  Vector Column(unsigned col) const { return isTransponate ? _Row(col) : _Column(col); };
  Vector Row(unsigned col)    const { return !isTransponate ? _Row(col) : _Column(col); };

  // Eigenvectoren und Eigenwerte einer Symetrischen Matrix
  // Matrix will be modified to get the result. So we need the original too here.
  // eigenvectoren in Q
  Vector SymEigen(Matrix&) const;

  // Singulaerwertzerlegung einer Matrix
  Vector SVD(Matrix &U,Matrix &V) const;
  void   SVD(Matrix &U,Matrix &S,Matrix &V) const;
  Vector SVD2(Matrix &U,Matrix &V) const;

  // Loesung eines Gleichungssystems (QR-Zerlegung)
  Vector Solve(const Vector &b) const;

#if defined(CHECK) || defined(DEBUG_CHECK)
  bool ok() const;
#endif

 protected:

  mutable int  theError;

  bool isTransponate;
  _MatrixData *theData;

  static Matrix &Hfactor(Matrix &,Vector &,Vector &);
  void Usolve(const Matrix &matrix,const Vector &b,Vector &out,double diag) const;

  Vector _Row(unsigned);
  Vector _Row(unsigned) const;
  Vector _Column(unsigned); 
  Vector _Column(unsigned) const;

  Vector TrieEigen(Vector&,Vector&,Matrix&) const;
  
  static Matrix &RotCols(Matrix&,unsigned,unsigned,double,double);

  // Memory management
  void RemoveData();
  void AddData(const Matrix &);
  void CopyData();
  void CreateData(const Matrix&);

  // Die QR-Zerlegung der Matrix Falls Solve() verwendet wurde
  mutable Matrix *theQRFactor;
  mutable Vector *theQRDiag;

#if defined(CHECK) || defined(DEBUG_CHECK)
  const char* theIdentifyer;
#endif
};


std::ostream &operator << (std::ostream&,const Matrix&);
std::ostream &operator << (std::ostream&,const Vector&);
// 
// // Not yet implemented:
// std::istream &operator >> (std::istream&,Matrix&);
// std::istream &operator >> (std::istream&,Vector&);

#endif
