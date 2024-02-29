#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"

class hit_record; // circular reference issue

/*
What do materials do?

When a ray cast from the camera hits an object, the material defines some set of graphical behavior for the incident ray.
    1. Is it absorbed completely?
    2. At what angle does it reflect? 
    3. How much does light does it reflect? 
    4. What color is it? 
    ETC ETC.

The ray produced by the material is called the scattered ray, 
because the incident ray's behavior is dependent on how the material scatters it

For this program, a material only does two things...
    1. Produce a scattered ray (or is it absorbed?)
    2. If scattered, say how much it should be attenuated

*/

// Abstract material, represent all material types as children
class material {
    public:
        virtual ~material() = default;

        virtual bool scatter(const ray& r_in, const hit_record& hit, color& attenuation, ray& scattered) const = 0;
};


// Lambertian (diffuse) material --> incident ray is scattered in many directions and attenuates by a ratio of R
class lambertian : public material {
    public:
        lambertian(const color& a) : albedo(a) {}

        bool scatter(const ray& r_in, const hit_record& hit, color& attenuation, ray& scattered) const override {
            auto scatter_direction = hit.normal + random_unit_vector();

            // catch a magnitude zero reflection vector and just reassign it as our hit_record's normal
            if (scatter_direction.near_zero()) {
                scatter_direction = hit.normal;
            }

            scattered = ray(hit.point, scatter_direction);
            attenuation = albedo;
            return true;
        }

    private:
        color albedo;
};

// Metal material --> incident ray is reflected symmetrically across a horizontal plane under the surface normal
class metal : public material {
    public:
        metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

        bool scatter(const ray& r_in, const hit_record& hit, color& attenuation, ray& scattered) const override {
            // scatter direction == v + 2b..... how to find b? --> -(v*n)*n ... since n is a unit vector projection of -v onto n formula
            // reflect formula = v - 2*dot(v,n)*n;
            // https://immersivemath.com/ila/ch03_dotproduct/ch03.html at 3.1
            // https://raytracing.github.io/images/fig-1.15-reflection.jpg (note that v points inwards, so we reflect it out)
            vec3 reflection = reflect(unit_vector(r_in.direction()), hit.normal);
            scattered = ray(hit.point, reflection + fuzz*random_unit_vector());
            attenuation = albedo;
            // if the normal between our fuzzed vector and surface normal is < 0, the surface just absorbs it
            return (dot(scattered.direction(), hit.normal) > 0);
        }

    private:
        color albedo;
        double fuzz;
};


/*
Dielectrics, i.e. water, glass, diamond, are materials that split colliding light rays into a reflected and a refracted ray.
    We will handle this, for a given incident ray, 
    by randomly choosing between reflection and refraction, 
    and generating one scattered ray

Refraction is described by Snell' Law
    n1 * sin(angle1) = n2 * sin(angle2)
    - angle1 & angle2 are angles from the surface normal
    - n1 & n2 are refractive indices (determined by specific material, air = 1.0, glass = 1.3-1.7)

Angle2 is the desired refraction angle

We can product a formula for the desired refraction angle using below train of logic 
https://raytracing.github.io/books/RayTracingInOneWeekend.html#metal/mirroredlightreflection
*/

class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}
        
        bool scatter(const ray& r_in, const hit_record& hit, color& attenuation, ray& scattered) const override {
            attenuation = color(1.0, 1.0, 1.0); // always 1, since the surface absorbs nothing
            double refraction_ratio = hit.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, hit.normal), 1.0);
            double sin_theta = sqrt(1 - cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;
            // inside of the dielectric object or angle of view mirror approximation holds
            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
                // reflect instead
                direction = reflect(unit_direction, hit.normal);
            }
            else {
                direction = refract(unit_direction, hit.normal, refraction_ratio);
            }
            scattered = ray(hit.point, direction);
            return true;
        }

    private:
        double ir;

        static double reflectance(double cosine, double ref_idx) { 
            // Schlick's approximation for reflectance
            auto r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0) * pow(1-cosine, 5);
        }
};

#endif