#ifndef _CAMERA_H
#define _CAMERA_H

#include "Utilities.h"

class Camera {
    public:
        Camera(Point3 lookfrom, Point3 lookat, Vec3 vup, int image_width, int image_height, double vfov, double aperture, double focus_dist) {
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto aspect_ratio = (double) image_width / (double)image_height;
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;

            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);
            
            origin = lookfrom;
            lookAt = lookat;
            camUp = vup;
            horizontal = focus_dist * viewport_width * u;
            vertical = focus_dist * viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;
            lens_radius = aperture / 2;
        }

        void moveCamera(Point3 positionOffset, Point3 camDirection, int image_width, int image_height, double vfov, double aperture, double focus_dist){
            origin += positionOffset;
            lookAt = camDirection + origin;

            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto aspect_ratio = (double) image_width / (double)image_height;
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;

            w = unit_vector(origin - lookAt);
            u = unit_vector(cross(camUp, w));
            v = cross(w, u);
            
            horizontal = focus_dist * viewport_width * u;
            vertical = focus_dist * viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;
            lens_radius = aperture / 2;
        }

        void rotateCamera (Point3 camDirection, int image_width, int image_height, double vfov, double aperture, double focus_dist){
            lookAt = camDirection + origin; // set camera look-at direction to the specified camdirection (which is basically an offset) plus the origin, which is its current pos

            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto aspect_ratio = (double) image_width / (double)image_height;
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;

            w = unit_vector(origin - lookAt);
            u = unit_vector(cross(camUp, w));
            v = cross(w, u);
            
            horizontal = focus_dist * viewport_width * u;
            vertical = focus_dist * viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;
            lens_radius = aperture / 2;
        }

        void updateFOV(int image_width, int image_height, double vfov, double aperture, double focus_dist) {
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto aspect_ratio = (double) image_width / (double)image_height;
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;

            w = unit_vector(origin - lookAt);
            u = unit_vector(cross(camUp, w));
            v = cross(w, u);
            horizontal = focus_dist * viewport_width * u;
            vertical = focus_dist * viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;
            lens_radius = aperture / 2;
        }

        Ray get_ray(double s, double t) const {
            Vec3 rd = lens_radius * random_in_unit_disk();
            Vec3 offset = u * rd.x() + v * rd.y();

            return Ray(
                origin + offset,
                lower_left_corner + s*horizontal + t*vertical - origin - offset
            );
        }

    private:
        Point3 origin;
        Point3 lookAt;
        Point3 lower_left_corner;
        Point3 camUp;
        Vec3 horizontal;
        Vec3 vertical;
        Vec3 u;
        Vec3 v; 
        Vec3 w;
        double lens_radius;
        
};
#endif