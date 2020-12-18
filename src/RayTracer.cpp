#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Vect.h"
#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Light.h"
//#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct RGBType {
    double r;
    double g;
    double b;
};

void savebmp (const char *filename, int w, int h, int dpi, RGBType *data) {
    FILE *f;
    int s = 4*w*h;
    int filesize = 54 + s;

    double factor = 39.375;
    int m = static_cast<int>(factor);
    
    int ppm = dpi*m;

    unsigned char bmpfileheader[14] = {'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};

    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize>>8);
    bmpfileheader[4] = (unsigned char)(filesize>>16);
    bmpfileheader[5] = (unsigned char)(filesize>>24);

    bmpinfoheader[4] = (unsigned char)(w);
    bmpinfoheader[5] = (unsigned char)(w>>8);
    bmpinfoheader[6] = (unsigned char)(w>>16);
    bmpinfoheader[7] = (unsigned char)(w>>24);

    bmpinfoheader[8] = (unsigned char)(h);
    bmpinfoheader[9] = (unsigned char)(h>>8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);

    bmpinfoheader[21] = (unsigned char)(s);
    bmpinfoheader[22] = (unsigned char)(s>>8);
    bmpinfoheader[23] = (unsigned char)(s>>16);
    bmpinfoheader[24] = (unsigned char)(s>>24);

    bmpinfoheader[25] = (unsigned char)(ppm);
    bmpinfoheader[26] = (unsigned char)(ppm>>8);
    bmpinfoheader[27] = (unsigned char)(ppm>>16);
    bmpinfoheader[28] = (unsigned char)(ppm>>24);

    bmpinfoheader[29] = (unsigned char)(ppm);
    bmpinfoheader[30] = (unsigned char)(ppm>>8);
    bmpinfoheader[31] = (unsigned char)(ppm>>16);
    bmpinfoheader[32] = (unsigned char)(ppm>>24);

    f = fopen(filename, "wb");

    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);

    for (int i = 0; i < w*h; i++) {
        RGBType rgb = data[i];
        double red      = (data[i].r) * 255;
        double green    = (data[i].g) * 255;
        double blue     = (data[i].b) * 255;

        unsigned char color[3] = {(int) floor(blue),(int) floor(green),(int) floor(red)};
        fwrite(color,1,3,f);
    }

    fclose(f);
}

int main (int argc, char* argv[]){
    cout << "rendering ..." << endl;

    // image related
    int dpi = 72;
    int width = 640;
    int height = 480;
    int n = width * height;
    RGBType *pixels = new RGBType[n];
    // 

    // basic vects
    Vect X (1,0,0);
    Vect Y (0,1,0);
    Vect Z (0,0,1);
    //

    // cam setup
    Vect camPosition (3, 1.5, -4);

    Vect look_at (0,0,0);
    Vect diff_btw (camPosition.getVectX() - look_at.getVectX(), camPosition.getVectY() - look_at.getVectY(), camPosition.getVectZ() - look_at.getVectZ());

    Vect camDirection = diff_btw.negative().normalize();
    Vect camRight = Y.crossProduct(camDirection).normalize();
    Vect camDown = camRight.crossProduct(camDirection);
    Camera sceneCam (camPosition, camDirection, camRight, camDown);
    // 

    // basic colors
    Color white (1.0, 1.0, 1.0, 0);
    Color gray (0.5, 0.5, 0.5, 0);
    Color black (0, 0, 0, 0);
    
    Color green (0.5, 1.0, 0.5, 0.3);
    //

    // light setup
    Vect lightPosition (-7, 10, -10);
    Light lightSource (lightPosition, white);
    //


    int currentPixel = 0;
    for(int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            currentPixel = y*width + x;
            pixels[currentPixel].r = 155;
            pixels[currentPixel].g = 155;
            pixels[currentPixel].b = 155;
        }
    }

    savebmp("scene.bmp", width, height, dpi, pixels);
}