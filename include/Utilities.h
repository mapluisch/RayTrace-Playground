#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>



// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

// source: https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
// a bit modified by me, using unix color codes from https://stackoverflow.com/questions/9158150/colored-output-in-c/9158263
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
#define BLUE            "\033[34m" 
#define BOLDYELLOW      "\033[1m\033[33m"
#define BOLDRED         "\033[1m\033[31m"
#define BOLDWHITE       "\033[1m\033[37m"  
#define RESET           "\033[0m"
void displayProgressbar(float progress) {
    int val = (int) (progress * 100);
    int lpad = (int) (progress * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r[%sRendering... %3d%%%s] [%s%.*s%*s%s]", BOLDYELLOW, val, RESET, BLUE, lpad, PBSTR, rpad, "", RESET);
    fflush(stdout);
}

// Common Headers
#include "Ray.h"
#include "Vec3.h"
#endif