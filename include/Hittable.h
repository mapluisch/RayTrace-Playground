#ifndef _HITTABLE_H
#define _HITTABLE_H

#include "Ray.h"
#include "Utilities.h"
#include "AABB.h"

class Material;

struct Hit {
    Point3 p;
    Vec3 normal;
    double t;
    double u;
    double v;
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
        virtual bool bounding_box(double time0, double time1, AABB& output_box) const = 0;
};

class Translate : public Hittable {
    public:
        Translate(shared_ptr<Hittable> p, const Vec3& displacement)
            : ptr(p), offset(displacement) {}

        virtual bool hit(
            const Ray& r, double t_min, double t_max, Hit& hit) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

    public:
        shared_ptr<Hittable> ptr;
        Vec3 offset;
};

bool Translate::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    Ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (!ptr->hit(moved_r, t_min, t_max, hit))
        return false;

    hit.p += offset;
    hit.set_face_normal(moved_r, hit.normal);

    return true;
}

bool Translate::bounding_box(double time0, double time1, AABB& output_box) const {
    if (!ptr->bounding_box(time0, time1, output_box))
        return false;

    output_box = AABB(
        output_box.min() + offset,
        output_box.max() + offset);

    return true;
}

class RotateY : public Hittable {
    public:
        RotateY(shared_ptr<Hittable> p, double angle);

        virtual bool hit(
            const Ray& r, double t_min, double t_max, Hit& hit) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
            output_box = bbox;
            return hasbox;
        }

    public:
        shared_ptr<Hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        AABB bbox;
};

RotateY::RotateY(shared_ptr<Hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    Point3 min( infinity,  infinity,  infinity);
    Point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i*bbox.max().x() + (1-i)*bbox.min().x();
                auto y = j*bbox.max().y() + (1-j)*bbox.min().y();
                auto z = k*bbox.max().z() + (1-k)*bbox.min().z();

                auto newx =  cos_theta*x + sin_theta*z;
                auto newz = -sin_theta*x + cos_theta*z;

                Vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = AABB(min, max);
}

bool RotateY::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta*r.origin()[0] - sin_theta*r.origin()[2];
    origin[2] = sin_theta*r.origin()[0] + cos_theta*r.origin()[2];

    direction[0] = cos_theta*r.direction()[0] - sin_theta*r.direction()[2];
    direction[2] = sin_theta*r.direction()[0] + cos_theta*r.direction()[2];

    Ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, hit))
        return false;

    auto p = hit.p;
    auto normal = hit.normal;

    p[0] =  cos_theta*hit.p[0] + sin_theta*hit.p[2];
    p[2] = -sin_theta*hit.p[0] + cos_theta*hit.p[2];

    normal[0] =  cos_theta*hit.normal[0] + sin_theta*hit.normal[2];
    normal[2] = -sin_theta*hit.normal[0] + cos_theta*hit.normal[2];

    hit.p = p;
    hit.set_face_normal(rotated_r, normal);

    return true;
}

#endif