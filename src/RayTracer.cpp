#include <iostream>
#include <fstream>      // used for writing image to file
#include <cxxopts.hpp>  // used for nicer argument parsing

#include "Utilities.h"
#include "Color.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"

Color ray_color(const Ray& r, const Hittable& world, int depth) {
    Hit hit;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0){
        return Color(0,0,0);
    }
    
    if (world.hit(r, 0.001, infinity, hit)) { //set t_min to 0.001 to avoid "shadow acne"
        Ray scattered;
        Color attenuation;
        if (hit.mat_ptr->scatter(r, hit, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return Color(0,0,0);
    }
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

int main(int argc, char* argv[]) {
    // Create CXXOPTS-Argument parser for nice argument input
    cxxopts::Options options("RayTracer", "A basic RayTracer, based on 'Ray Tracing In One Weekend' by Peter Shirley, adapted and extended by Martin Pluisch.");
    options.add_options()
        ("h,help", "Print usage information")
        ("x,width", "Image width in pixel", cxxopts::value<int>())
        ("y,height", "Image height in pixel", cxxopts::value<int>())
        ("s,samples_per_pixel", "Samples per Pixel", cxxopts::value<int>())
        ("d,depth", "Maximum depth", cxxopts::value<int>())
        ("o,output", "Output file name", cxxopts::value<std::string>())
    ;
    auto result = options.parse(argc, argv);

    if (result.count("help") == 1) {
      std::cout << options.help() << std::endl;
      exit(0);
    }
    
    // Open and overwrite image file
    std::ofstream output_image;
    std::string output_filename = (result.count("output") ? result["output"].as<std::string>() : "output.ppm");
    output_image.open(output_filename);

    // Output-Image dimensions
    // Read and store passed in arguments or default to 16:9-resolution with 50 samples per pixel
    const int image_width       = (result.count("width") ? result["width"].as<int>() : 400);
    const int image_height      = (result.count("height") ? result["height"].as<int>() : 225);
    const int samples_per_pixel = (result.count("samples_per_pixel") ? result["samples_per_pixel"].as<int>() : 50);
    const int max_depth         = (result.count("depth") ? result["depth"].as<int>() : 50);

    // World
    Hittable_List world;
    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.7, 0.3, 0.3));
    auto material_left   = make_shared<Metal>(Color(0.8, 0.8, 0.8));
    auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2));

    world.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
    world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right));

    // Camera
    Camera cam(image_width, image_height);

    // Render
    std::cerr << "-- start rendering --\n";
    output_image << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\r-- scanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(output_image, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\n-- done rendering, closing output file-stream --\n";
    // close output-image filestream
    output_image.close();
}