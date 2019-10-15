#ifndef Vect_h
#define Vect_h

// slightly more functions than math.h
#include <cmath>

using namespace std;

// To help the make math easier
class _3d_values {
public:
    // too late to refactor these into private, but these are just the x, y, z coordinates for implementing math
    float X, Y, Z;
    // constructor
    _3d_values(float x, float y, float z) : X(x), Y(y), Z(z) {}
    _3d_values() {}
    // + operation for two vectors
    _3d_values operator + (_3d_values vector) {
        return _3d_values(X+vector.X,Y+vector.Y,Z+vector.Z);
    }
    // - operation for two vectors
    _3d_values operator -(_3d_values vector) {
        return _3d_values(X-vector.X,Y-vector.Y,Z-vector.Z);
    }

    // * operation for two scalar values
    _3d_values operator * (float scalar) {
        return _3d_values(scalar*X,scalar*Y,scalar*Z);
    }

    // * operation for two vectors [override]
    _3d_values operator * (_3d_values vector) {
        return _3d_values(vector.X*X,vector.Y*Y,vector.Z*Z);
    }

    // cross product
    _3d_values cross(_3d_values vector) {
        return _3d_values(Y*vector.Z-Z*vector.Y,
                Z*vector.X - X*vector.Z,
                X*vector.Y - Y*vector.X);
    }

    // dor product
    float dot(_3d_values vector) {
        return X*vector.X+Y*vector.Y+Z*vector.Z;
    }

    // calculate magnitude
    float magnitude () {
        return sqrt(dot(*this));
    }

    bool operator <( const _3d_values &rhs) const{
       return (Z < rhs.Z);
    }

    /*float magnitude (_3d_values vector) {
        return sqrt(dot(vector));
    }*/

    // normalize vector by dividing with magnitude
    _3d_values normalize() {
        return (*this)*(1/magnitude());
    }

    // angle between two vectors
    float cal_angle(_3d_values vector) {
        return acos((this->dot(vector))/(this->magnitude()*vector.magnitude()));
    }
};

class _2d_values {
public:
    int X, Y;
    _2d_values() {}
    _2d_values(int x, int y) : X(x), Y(y){}
    // scalar
    _2d_values operator * (float scalar) {
        return _2d_values(scalar*X,scalar*Y);
    }
    // vector
    _2d_values operator * (_3d_values vector) {
        return _2d_values(vector.X*X,vector.Y);
    }
};

#endif
