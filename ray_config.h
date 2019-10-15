#ifndef Ray_h
#define Ray_h

#include "_3d_values.h"

// We would like to have the ray separate from others so it's easier to control
class Ray_Vector {
    // Encapsulate variables
private:
    // origin and direction
    _3d_values origin;
    _3d_values direction;
public:
    // set the constructor
    Ray_Vector(_3d_values o, _3d_values d) : origin(o), direction(d) {}
    // getters
    _3d_values get_origin() {
        return origin;
    }
    _3d_values get_direction() {
        return direction;
    }
};
#endif
