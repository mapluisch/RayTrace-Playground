#include <iostream>
#include <fstream>      // used for writing image to file
#include <cxxopts.hpp>  // used for nicer argument parsing

#include "Utilities.h"
#include "Color.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"

#include <opencv2/opencv.hpp>
using namespace cv;

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

Hittable_List random_scene() {
    Hittable_List world;

    auto ground_material = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    world.add(make_shared<Sphere>(Point3(0,-1000,0), 1000, ground_material));

    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<Material> sphere_material;

                if (choose_mat < 0.6) {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = make_shared<Lambertian>(albedo);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    return world;
}


int main(int argc, char* argv[]) {
    // CXXOPTS Init and Parsing
    // Create CXXOPTS-Argument parser for nice argument input
    cxxopts::Options options("RayTracer", "A basic RayTracer, based on 'Ray Tracing In One Weekend' by Peter Shirley, adapted and extended by Martin Pluisch.");
    options.add_options()
        ("h,help", "Print usage information")
        ("x,width", "Image width in pixel", cxxopts::value<int>())
        ("y,height", "Image height in pixel", cxxopts::value<int>())
        ("s,samples_per_pixel", "Samples per Pixel", cxxopts::value<int>())
        ("d,depth", "Maximum depth", cxxopts::value<int>())
        ("f,fov", "Camera's vertical field of view", cxxopts::value<double>())
        ("o,output", "Output file name", cxxopts::value<std::string>())
        ("r,random", "Boolean whether to create a random scene-world or not", cxxopts::value<bool>())
    ;
    auto result = options.parse(argc, argv);

    if (result.count("help") == 1) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // CXXOPTS Parsing
    // Read and store passed in arguments or default to 16:9-resolution with 50 samples per pixel
    const int image_width       = (result.count("width") ? result["width"].as<int>() : 400);
    const int image_height      = (result.count("height") ? result["height"].as<int>() : 225);
    const int samples_per_pixel = (result.count("samples_per_pixel") ? result["samples_per_pixel"].as<int>() : 50);
    const int max_depth         = (result.count("depth") ? result["depth"].as<int>() : 50);
    const double vfov           = (result.count("fov") ? result["fov"].as<double>() : 90);
    const bool randomize_world  = (result.count("random") ? result["random"].as<bool>() : false);
    //

    // Open and overwrite image file
    std::ofstream output_image;
    std::string output_filename = (result.count("output") ? result["output"].as<std::string>() : "output.ppm");
    output_image.open(output_filename);

    
    

    // World
    Hittable_List world;
    if(randomize_world){
        world = random_scene();
    } else {
        // use predefined scene
        auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
        auto material_center = make_shared<Dielectric>(1.5);
        auto material_left   = make_shared<Metal>(Color(0.2, 0.4, 0.8), 0.3);
        auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);

        world.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
        world.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
        world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
        world.add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right)); 
    }


    // Camera
    Point3 camPosition (-1,1,1);
    Point3 camDirection (0,0,-1);
    Vec3 camUp (0,1,0);
    double camAperture = 0.0;
    double camFocusDistance = (camPosition-camDirection).length();
    Camera cam(camPosition, camDirection, camUp, image_width, image_height, vfov, camAperture, camFocusDistance);

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

    #pragma region OpenCV testing ------------------------------

    cv::Mat input = cv::imread("output.ppm", IMREAD_COLOR);
    namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
     imshow("Display window", input);
    cv::waitKey(0);
    #pragma endregion
}