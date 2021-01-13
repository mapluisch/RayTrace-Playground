#ifndef _XYRECT_H
#define _XYRECT_H

#include "Utilities.h"
#include "Hittable.h"

class XY_Rect : public Hittable {
    public:
        XY_Rect() {}
        // k = Abstand auf Z-Achse
        XY_Rect(double _x0, double _x1, double _y0, double _y1, double _k, shared_ptr<Material> mat) : 
            x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& hit) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Z
            // dimension a small amount.
            output_box = AABB(Point3(x0,y0, k-0.0001), Point3(x1, y1, k+0.0001));
            return true;
        }

    public:
        shared_ptr<Material> mp;
        double x0, x1, y0, y1, k;
};

class XZ_Rect : public Hittable {
    public:
        XZ_Rect() {}

        XZ_Rect(double _x0, double _x1, double _z0, double _z1, double _k, shared_ptr<Material> mat) :
            x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& hit) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Y
            // dimension a small amount.
            output_box = AABB(Point3(x0,k-0.0001,z0), Point3(x1, k+0.0001, z1));
            return true;
        }

    public:
        shared_ptr<Material> mp;
        double x0, x1, z0, z1, k;
};

class YZ_Rect : public Hittable {
    public:
        YZ_Rect() {}

        YZ_Rect(double _y0, double _y1, double _z0, double _z1, double _k, shared_ptr<Material> mat) : 
            y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& rec) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the X
            // dimension a small amount.
            output_box = AABB(Point3(k-0.0001, y0, z0), Point3(k+0.0001, y1, z1));
            return true;
        }

    public:
        shared_ptr<Material> mp;
        double y0, y1, z0, z1, k;
};

bool XY_Rect::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    auto t = (k-r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto y = r.origin().y() + t*r.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    hit.u = (x-x0)/(x1-x0);
    hit.v = (y-y0)/(y1-y0);
    hit.t = t;
    auto outward_normal = Vec3(0, 0, 1);
    hit.set_face_normal(r, outward_normal);
    hit.mat_ptr = mp;
    hit.p = r.at(t);
    return true;
}

bool XZ_Rect::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    auto t = (k-r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto z = r.origin().z() + t*r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    hit.u = (x-x0)/(x1-x0);
    hit.v = (z-z0)/(z1-z0);
    hit.t = t;
    auto outward_normal = Vec3(0, 1, 0);
    hit.set_face_normal(r, outward_normal);
    hit.mat_ptr = mp;
    hit.p = r.at(t);
    return true;
}

bool YZ_Rect::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    auto t = (k-r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin().y() + t*r.direction().y();
    auto z = r.origin().z() + t*r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    hit.u = (y-y0)/(y1-y0);
    hit.v = (z-z0)/(z1-z0);
    hit.t = t;
    auto outward_normal = Vec3(1, 0, 0);
    hit.set_face_normal(r, outward_normal);
    hit.mat_ptr = mp;
    hit.p = r.at(t);
    return true;
}

#endif