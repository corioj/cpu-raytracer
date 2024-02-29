#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>

class camera {
    public:
        double aspect_ratio = 1.0; // Ratio of image width over image height
        int image_width = 100; // Width of the image generated
        int samples_per_pixel = 10; // Count of random samples for each pixel
        int max_depth = 10; // Maximum number of ray bounces into scene 
        // {RAY BOUNCE: Recursive because if we've bounced off n objects, it's another bounce (repeated math) on the n+1th object}

        double vfov = 90; // vertical view angle (field of view)

        point3 lookfrom = point3(0, 0, -1); // point camera is looking from
        point3 lookat   = point3(0, 0, 0); // point camera is looking at
        vec3   vup      = vec3(0, 1, 0); // camera-relative up direction

        // defocus blur
        /*
            Essentially, create a disc around the camera center (lookfrom), 
            which can receive rays from a given pixel on the defocus disc (camera lens)

            For this we need a defocus angle and a distance, denoting the disc from which a ray out of a given pixel 
            can hit the disc

            The defocus angle denotes the greatest angle between a ray from the camera lens towards a given pixel
            Defocus distance denotes the distance from camera center to the pixel orthogonal to lookfrom plane *13.1: FIGURE 22*
        */
        double defocus_angle = 0; // variation angle of rays through each pixel
        double focus_dist = 10; // distance from camera lookfrom point to plane of perfect focus

        void render(const hittable& world) {
            // initialize
            initialize();

            // Render
            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; ++j) {
                // progress logging
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                // pixel by pixel, shoot out rays into the world that map to a pixel location
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; ++sample) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    write_color(std::cout, pixel_color, samples_per_pixel);
                }
            }

            // progress logging
            std::clog << "\rDone.                  \n" << std::flush;
        }

    private:
        int    image_height; // height of image
        point3 center; // camera center
        point3 pixel00_loc; // location of upper-left pixel
        vec3   pixel_delta_u; // offset to pixel to the right 
        vec3   pixel_delta_v; // offset to pixel below

        vec3 u, v, w; // camera frame basis vectors, X, Y, Z ordering

        vec3 defocus_disk_u; // defocus disk horizontal radius
        vec3 defocus_disk_v; // defocus disk vertical radius

        void initialize() {
            // image_height
            image_height = static_cast<int>(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;

            // set center
            center = lookfrom;

            // determine viewport dimensions
            //auto focal_length = (lookfrom - lookat).length();
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto viewport_height = 2 * h * focus_dist; // focal length replacement
            auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

            // calculate the camera frame basis vectors (RIGHT HAND RULE, CROSS, GO OVER!!!)
            w = unit_vector(lookfrom - lookat); // z-direction
            u = unit_vector(cross(vup, w)); // x-direction
            v = cross(w, u); // y-direction

            // calculate horizontal and vertical viewport vectors
            vec3 viewport_u = viewport_width * u; // vector across viewport horizontal edge
            vec3 viewport_v = viewport_height * -v; // vector down viewport vertical edge

            // calculate horizontal and vertical delta vectors from pixel to pixel
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height; 

            // calculate location of the upper-left pixel
            auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            // calculate the camera defocus disk basis vectors
            auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            hit_record hit;

            // reached max depth of recursive ray bounces, generate no further color
            if (depth <= 0) {
                return color(0,0,0);
            }

            // generate light on another surface from ray bouncing
            if (world.hit(r, interval(0.001, infinity), hit)) { // 0.001 to avoid self-intersections with a previously hit point on a surface
                ray scattered;
                color attenuation;
                // scatter light from a ray bounce
                if (hit.mat->scatter(r, hit, attenuation, scattered) == true) {
                    return attenuation * ray_color(scattered, depth-1, world);
                }

                // we didn't hit another surface, do not generate any more light on a given point
                return color(0,0,0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        }

        ray get_ray(int i, int j) {
            // Get a randomly sampled camera ray for the pixel at location i,j, originating from the defocus disk
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto pixel_sample = pixel_center + pixel_sample_square();

            // if our defocus angle is not 0, then we have a defocus disc (lens) from which we generate rays
            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 pixel_sample_square() const {
            // Return a random point in a square surrounding a pixel at the origin
            auto px = -0.5 + random_double();
            auto py = -0.5 + random_double();
            return (px * pixel_delta_u) + (py * pixel_delta_v);
        }
        
        point3 defocus_disk_sample() const {
            // returns a random point on the defocus disk
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
};

#endif