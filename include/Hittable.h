#ifndef _HITTABLE_H
#define _HITTABLE_H

#include "Ray.h"
#include "Utilities.h"

class Material;

struct Hit {
    Point3 p;
    Vec3 normal;
    double t;
    shared_ptr<Material> mat_ptr;
    bool front_face;

    inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal :-outward_normal;
    }
};

class Hittable {
    public:
        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& rec) const = 0;
};

#endif