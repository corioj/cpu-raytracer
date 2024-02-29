#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "camera.h"

int main() {
    hittable_list world;

    auto material_ground = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3( 0.0, -1000, 0.0), 1000, material_ground));

    // generate some random spheres
    for (int x = -5; x < 5; x++) {
        for (int y = -5; y < 5; y++) {
            // generate a random center point for a sphere, and a random material choice
            auto choose_material = random_double();
            point3 center(x + 0.9*random_double(), 0.2, y + 0.4*random_double());

            // if we are outside of our silly big example balls aligned along x=4
            if ((center - point3(4, 1, 0)).length() > 1) {
                shared_ptr<material> sphere_material;

                // 75% chance for lambertian
                if (choose_material < 0.75) {
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                // 20% chance for metal
                else if (choose_material < 0.95) {
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                // 5% chance of dielectric surface
                else {
                    sphere_material = make_shared<dielectric>(1.2);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    // make big balls
    auto material1 = make_shared<lambertian>(color(0.7, 0.3, 0.2));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material1));

    auto material2 = make_shared<metal>(color(0.4, 0.7, 0.1), 0.0);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material2));

    auto material3 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // Render the World //
    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 25;

    cam.vfov     = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat   = point3(0, 0, 0);
    cam.vup      = vec3(0, 1, 0);

    cam.defocus_angle = 1.0;
    cam.focus_dist    = 10.0;

    cam.render(world);
}