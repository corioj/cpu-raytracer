#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

/*
Data structure designed to help create object-order rendering in the main program
*/
class hittable_list : public hittable {
    public:
        std::vector<shared_ptr<hittable>> objects;

        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); } 

        void clear() { 
            objects.clear(); 
        }

        void add(shared_ptr<hittable> object) { 
            objects.push_back(object); 
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            hit_record temp_rec;
            bool hit_anything = false;
            auto closest_so_far = ray_t.max;

            // check for closest hit and update the hit record
            for (const auto& object : objects) {
                if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            // return the hit status
            return hit_anything;
        }
};

/* Notes about shared_ptr:
shared_ptr<type> is a reference to some object, with reference-counting semantics

every time you assign its value to another shared pointer, the reference counter goes up
any time it goes out of scope, the reference counter goes down

smart pointers are safely deleted when the reference counter goes down to 0

typically initialized using shared_ptr<type> type_ptr = make_shared<type>(---double, for example---);

we'll use shared pointers in our code, because it allows multiple geometries to share a common instance,
for example, a bunch of spheres that all use the same color material, 
and because it makes memory management automatic and easier to reason about

*/

#endif