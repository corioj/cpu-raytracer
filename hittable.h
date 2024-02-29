#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"

class material; // circular reference issue

// Exact point, normal, and t value on our hittable that the current ray cast from the camera intersects
class hit_record {
    public:
        point3 point;
        vec3 normal;
        double t;

        // instance of surface material that can be shared by many hits 
        shared_ptr<material> mat; 

        // remembering which side of the surface was hit (normals point against the ray implementation)
        bool front_face;

        void set_face_normal(const ray& r, const vec3& outward_normal) {
            // outward_normal assumed to be unit_length (why is this important ?)

            // take the dot product with surface outward normal and raycast to determine side of intersection
            front_face = dot(r.direction(), outward_normal) < 0.0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};

// What if we want to "hit" more than just spheres? Abstract class for anything a ray might hit...
// Then, regardless of it it's a sphere, or an array of spheres, or other objects, it's simple to generate a hit
class hittable {
    public:
        virtual ~hittable() = default;
        virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

/* NOTES:
We can set things up so that normals always point outward from the world object surface, 
or always point against the incident (intersecting) ray. 

If we decide to have the normals always point against the ray, we won't be able to use the dot product 
to determine which side of the surface the ray is on. Instead, we would need to store that information

If we decide to have the normals always point out, then we will need to determine which side 
the ray is on when we color it. This can be determined by taking the dot product of the two vectors, 
where if their dot is positive, the ray is inside the sphere.
*/

#endif