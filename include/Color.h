#ifndef _COLOR_H
#define _COLOR_H

#include "math.h"

class Color {
    double red, green, blue, special;

    public:
        Color();
        Color(double, double, double, double);
        // methods
        double getColorRed() {return red;}
        double getColorGreen() {return green;}
        double getColorBlue() {return blue;}
        double getColorSpecial() {return special;}

        void setColorRed(double r){red = r;}
        void setColorGreen(double g){green = g;}
        void setColorBlue(double b){blue = b;}
        void setColorSpecial(double s){special = s;}
};

Color::Color () {
    red = green = blue = 0.5;
}

Color::Color (double r, double g, double b, double s) {
    red = r;
    green = g;
    blue = b;
    special = s;
}

#endif