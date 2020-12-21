#ifndef _HITTABLE_LIST_H
#define _HITTABLE_LIST_H

#include "Hittable.h"
#include "AABB.h"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class Hittable_List : public Hittable {
    public:
        Hittable_List() {}
        Hittable_List(shared_ptr<Hittable> object) { add(object); }

        void clear() { objects.clear(); }
        void add(shared_ptr<Hittable> object) { objects.push_back(object); }

        virtual bool hit(const Ray& r, double t_min, double t_max, Hit& rec) const override;
        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

    public:
        std::vector<shared_ptr<Hittable>> objects;
};

bool Hittable_List::hit(const Ray& r, double t_min, double t_max, Hit& rec) const {
    Hit temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool Hittable_List::bounding_box(double time0, double time1, AABB& output_box) const {
    if (objects.empty()) return false;

    AABB temp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, temp_box)) return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }

    return true;
}

#endif