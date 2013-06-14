#ifndef VECTORMATH_H_
#define VECTORMATH_H_

#include <iostream>

// Physics and math constants
const static double PI = 3.141592653589f;
const static double G = 6.67e-11f;

// Vectors
typedef double Vector;
typedef Vector Force;
typedef Vector Acceleration;
typedef Vector Position;
typedef Vector Velocity;

typedef struct Vector3D {
   Vector x;
   Vector y;
   Vector z;
};
typedef Vector3D Force3D;
typedef Vector3D Acceleration3D;
typedef Vector3D Velocity3D;
typedef Vector3D Position3D;

// Scalar
typedef double Scalar;
typedef Scalar Mass;
typedef Scalar Time;


bool testVectorMath( void ) {

   return true;
}

#endif
