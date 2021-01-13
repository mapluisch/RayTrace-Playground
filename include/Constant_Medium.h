#ifndef _CONSTANT_MEDIUM_H
#define _CONSTANT_MEDIUM_H

#include "Utilities.h"

#include "Hittable.h"
#include "Material.h"
#include "Texture.h"

class Constant_Medium : public Hittable {
    public:
        Constant_Medium(shared_ptr<Hittable> b, double d, shared_ptr<Texture> a) : 
            boundary(b),
            neg_inv_density(-1/d),
            phase_function(make_shared<Isotropic>(a)) {}

        Constant_Medium(shared_ptr<Hittable> b, double d, Color c) : 
            boundary(b),
            neg_inv_density(-1/d),
            phase_function(make_shared<Isotropic>(c)) {}

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& hit) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
            return boundary->bounding_box(time0, time1, output_box);
        }

    public:
        shared_ptr<Hittable> boundary;
        shared_ptr<Material> phase_function;
        double neg_inv_density;
};

bool Constant_Medium::hit(const Ray& r, double t_min, double t_max, Hit& hit) const {
    // Print occasional samples when debugging. To enable, set enableDebug true.
    const bool enableDebug = false;
    const bool debugging = enableDebug && random_double() < 0.00001;

    Hit hit1, hit2;

    if (!boundary->hit(r, -infinity, infinity, hit1))
        return false;

    if (!boundary->hit(r, hit1.t+0.0001, infinity, hit2))
        return false;

    if (debugging) std::cerr << "\nt_min=" << hit1.t << ", t_max=" << hit2.t << '\n';

    if (hit1.t < t_min) hit1.t = t_min;
    if (hit2.t > t_max) hit2.t = t_max;

    if (hit1.t >= hit2.t)
        return false;

    if (hit1.t < 0)
        hit1.t = 0;

    const auto ray_length = r.direction().length();
    const auto distance_inside_boundary = (hit2.t - hit1.t) * ray_length;
    const auto hit_distance = neg_inv_density * log(random_double());

    if (hit_distance > distance_inside_boundary)
        return false;

    hit.t = hit1.t + hit_distance / ray_length;
    hit.p = r.at(hit.t);

    if (debugging) {
        std::cerr << "hit_distance = " <<  hit_distance << '\n'
                  << "hit.t = " <<  hit.t << '\n'
                  << "hit.p = " <<  hit.p << '\n';
    }

    hit.normal = Vec3(1,0,0);  // arbitrary
    hit.front_face = true;     // also arbitrary
    hit.mat_ptr = phase_function;

    return true;
}

#endif