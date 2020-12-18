#ifndef _RAY_H
#define _RAY_H

#include "math.h"
#include "Vect.h"

class Ray {
    Vect origin, direction;

    public:
        Ray();
        Ray(Vect, Vect);
        // methods
        Vect getRayOrigin () { return origin; }
        Vect getRayDirection () { return direction; }
};

Ray::Ray () {
    origin = Vect(0,0,0);
    direction = Vect(1,0,0);
}

Ray::Ray (Vect _origin, Vect _direction) {
    origin = _origin;
    direction = _direction;
}

#endif