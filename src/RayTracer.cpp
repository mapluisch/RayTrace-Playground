#include <iostream>
#include <fstream>      // used for writing image to file
#include <cxxopts.hpp>  // used for nicer argument parsing

#include "Utilities.h"
#include "Color.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"
#include <math.h>

#include <opencv2/opencv.hpp>
using namespace cv;

int image_width;
 int image_height;
 int samples_per_pixel;
 int max_depth;
 double vfov;
 bool randomize_world;
 Hittable_List world;

 struct RGB {
    uchar blue;
    uchar green;
    uchar red;  };

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
    world.add(make_shared<Sphere>(Point3(0,-1000.5,0), 1000, ground_material));

    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9*random_double(), 1, b + 0.9*random_double());

            if ((center - Point3(4, 0, 0)).length() > 0.9) {
                shared_ptr<Material> sphere_material;

                if (choose_mat < 0.6) {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = make_shared<Lambertian>(albedo);
                    world.add(make_shared<Sphere>(center, 1, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 1, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 1, sphere_material));
                }
            }
        }
    }

    return world;
}




void renderScene(Camera& cam, Mat& image){
    for (int j = image_height-1; j >= 0; --j) {
        float progress = (float) (image_height - j)/ (float) image_height;
        displayProgressbar(progress);
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            auto r = pixel_color.x();
            auto g = pixel_color.y();
            auto b = pixel_color.z();
            //Divide the color by the number of samples and gamma-correct for gamma=2.0.
            auto scale = 1.0 / samples_per_pixel;
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);    

            RGB& rgb = image.ptr<RGB>(image_height - j)[i];
            rgb.blue = b*255; rgb.green = g*255; rgb.red = r*255;
        }
    }
}

void printExploreWelcomeMessage(){
    // show short intro message
    std::cout << BOLDRED << "Welcome to my BasicRayTracer!" << RESET << std::endl;
    std::cout << BOLDWHITE << "[W-A-S-D / arrow-keys to move and rotate, 1-2 to move up and down, Q-E to change the FOV, SPACE to render, ESC to quit]\n" << RESET << std::endl;
}

void setLowQualityRender() {
    samples_per_pixel = 2;
    max_depth = 5;
    image_width = 400;
    image_height = 225;
}

void setHighQualityRender() {
    samples_per_pixel = 100;
    max_depth = 20;
    image_width = 1280;
    image_height = 720;
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
        ("e,explore", "Boolean whether to be able to navigate through the scene", cxxopts::value<bool>())
    ;
    auto result = options.parse(argc, argv);

    if (result.count("help") == 1) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // CXXOPTS Parsing
    // Read and store passed in arguments or default to 16:9-resolution with 50 samples per pixel
    image_width         = (result.count("width") ? result["width"].as<int>() : 400);
    image_height        = (result.count("height") ? result["height"].as<int>() : 225);
    samples_per_pixel   = (result.count("samples_per_pixel") ? result["samples_per_pixel"].as<int>() : 50);
    max_depth           = (result.count("depth") ? result["depth"].as<int>() : 50);
    vfov                = (result.count("fov") ? result["fov"].as<double>() : 90);
    randomize_world     = (result.count("random") ? result["random"].as<bool>() : false);
    bool explore        = (result.count("explore") ? result["explore"].as<bool>() : false);
    //
    
    if(explore){
        printExploreWelcomeMessage();
    }

    // Exploration vars
    // hardcoded possible cam rotations (= cam lookat directions)
    float rotationAngle = 10;
    float exploreStepSize = 0.1; // units to move per arrow-keypress
    float vfovStep = 1;
    // Generate Camera
    Point3 currentCamPosition (0,0,0);
    Point3 camDirection = currentCamPosition + Vec3(0,0,-1);
    Vec3 camUp (0,1,0);
    double camAperture = 0.0;
    double camFocusDistance = (currentCamPosition-camDirection).length();
    Camera cam(currentCamPosition, camDirection, camUp, image_width, image_height, vfov, camAperture, camFocusDistance);


    // Generate World    
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

    // create image cv2-mat
    Mat image (image_height, image_width, CV_8UC3, Scalar(0,0,0));    
    // overwrite standard render-quality settings for faster first-frame rendering in explore mode
    if(explore) setLowQualityRender();
    // render initial scene-image
    renderScene(cam, image);

    #pragma region World-Exploring through movement, displayed in OpenCV window
    if(explore){        
        while(explore){
            // set low-quality setting for faster rendering
            setLowQualityRender();      
            
            namedWindow("RayTracer", WINDOW_AUTOSIZE);
            imshow("RayTracer", image);
            
            char key = cv::waitKey(0);

            if(key == 'e'){
                vfov -= vfovStep;
                cam.updateFOV(image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 'q'){
                vfov += vfovStep;
                cam.updateFOV(image_width, image_height, vfov, camAperture, camFocusDistance);
            }

            if(key == '1'){
                cam.moveCamera(currentCamPosition - Vec3(0,exploreStepSize,0), camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if(key == '2'){
                cam.moveCamera(currentCamPosition + Vec3(0,exploreStepSize,0), camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            }

            if (key == 2||key == 'a'||key == 'A'){ // left arrow
                camDirection.rotateAroundPoint(currentCamPosition, -rotationAngle);
                cam.rotateCamera(camDirection,image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 0||key == 'w'||key == 'W'){ // up arrow or uppper/lower-case w
                cam.moveCamera(camDirection * exploreStepSize, camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 3||key == 'd'||key == 'D'){ // right arrow
                camDirection.rotateAroundPoint(currentCamPosition, rotationAngle);
                cam.rotateCamera(camDirection,image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 1||key == 's'||key == 'S'){ // down arrow
                cam.moveCamera(camDirection * -exploreStepSize, camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 32){ // spacebar -> render image in 1080p
                setHighQualityRender();
                resize(image, image, cv::Size(image_width, image_height));
                resizeWindow("RayTracer", image_width, image_height);
            } else if (key == 27){ // escape, end loop
                explore = false;
            }
            renderScene(cam, image);
        }   
    } else {
        namedWindow("RayTracer", WINDOW_AUTOSIZE);// Create a window for display.
        imshow("RayTracer", image);    
        cv::waitKey(0);
    }
    #pragma endregion
}