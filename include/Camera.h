#ifndef _CAMERA_H
#define _CAMERA_H

#include "math.h"
#include "Vect.h"

class Camera {
    Vect  camPosition, camDirection, camRight, camDown;

    public:
        Camera();
        Camera(Vect, Vect, Vect, Vect);
        // methods
        Vect getCameraPosition () { return camPosition; }
        Vect getCameraDirection () { return camDirection; }   
        Vect getCameraRight () { return camRight; }
        Vect getCameraDown () { return camDown; }
};

Camera::Camera () {
    camPosition = Vect(0,0,0);
    camDirection = Vect(0,0,1);
    camRight = Vect(0,0,0);
    camDown = Vect(0,0,0);
}

Camera::Camera (Vect _position, Vect _direction, Vect _right, Vect _down) {
    camPosition = _position;
    camDirection = _direction;
    camRight = _right;
    camDown = _down;
}

#endif