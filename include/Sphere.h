#ifndef _SPHERE_H
#define _SPHERE_H

#include "Hittable.h"
#include "Vec3.h"
#include "AABB.h"

class Sphere : public Hittable {
    public:
        Sphere() {}
        Sphere(Point3 cen, double r, shared_ptr<Material> m) : center(cen), radius(r), mat_ptr(m) {};

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& rec) const override;
        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

    public:
        Point3 center;
        double radius;
        shared_ptr<Material> mat_ptr;
    
    private:
        static void get_sphere_uv(const Point3& p, double& u, double& v) {
            auto theta = acos(-p.y());
            auto phi = atan2(-p.z(), p.x()) + pi;

            u = phi / (2*pi);
            v = theta / pi;
        }
};

bool Sphere::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    Vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    hit.t = root;
    hit.p = r.at(hit.t);
    Vec3 outward_normal = (hit.p - center) / radius;
    hit.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, hit.u, hit.v);
    hit.mat_ptr = mat_ptr;

    return true;
}

bool Sphere::bounding_box(double time0, double time1, AABB& output_box) const {
    output_box = AABB(
        center - Vec3(radius, radius, radius),
        center + Vec3(radius, radius, radius));
    return true;
}

#endif