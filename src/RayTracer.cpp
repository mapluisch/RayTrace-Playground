#include <iostream>
#include <fstream>      // used for writing image to file
#include <cxxopts.hpp>  // used for nicer argument parsing
#include <math.h>
#include <omp.h>

#include "Utilities.h"

#include "Color.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"
#include "AARect.h"
#include "Box.h"

#include <opencv2/opencv.hpp>
using namespace cv;

int image_width;
int image_height;
int samples_per_pixel;
int max_depth;
double vfov;
bool randomize_world;
Hittable_List world;
Color background(0,0,0);
int currentSceneIndex = 0;


struct RGB {uchar blue; uchar green; uchar red;};

Color ray_color(const Ray& r, const Color& background, const Hittable& world, int depth) {
    Hit hit;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0){
        return Color(0,0,0);
    }

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, hit))
        return background;

    Ray scattered;
    Color attenuation;
    Color emitted = hit.mat_ptr->emitted(hit.u, hit.v, hit.p);

    if (!hit.mat_ptr->scatter(r, hit, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
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

Hittable_List modified_cornell() {
    Hittable_List objects;

    auto red    = make_shared<Lambertian>(Color(.65, .05, .05));
    auto blue   = make_shared<Lambertian>(Color(.35, .35, 1));
    auto white  = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green  = make_shared<Lambertian>(Color(.12, .45, .15));
    auto yellow = make_shared<Lambertian>(Color(0.9, 0.8, 0.1));
    auto light  = make_shared<Diffuse_Light>(Color(1,1,1));
    auto mirror = make_shared<Metal>(Color(0.7,0.7,0.7), 0.0);
    auto metal  = make_shared<Metal>(Color(0.2, 0.4, 0.8), 0.3);
    auto glass  = make_shared<Dielectric>(1.5);
    auto pertext = make_shared<Noise_Texture>(0.1);


    // left
    objects.add(make_shared<YZ_Rect>(-5, 5, -10, 0, -5, green));
    // right
    objects.add(make_shared<YZ_Rect>(-5, 5, -10, 0, 5, yellow));
    // top
    objects.add(make_shared<XZ_Rect>(-5, 5, -10, 0, 5, red));
    // bottom
    objects.add(make_shared<XZ_Rect>(-5, 5, -10, 0, -5, red));
    // back
    objects.add(make_shared<XY_Rect>(-5, 5, -5, 5, -10, white));
    // front
    objects.add(make_shared<XY_Rect>(-5, 5, -5, 5, 0.1, white));

    // light top
    objects.add(make_shared<XZ_Rect>(-0.5, 0.5, -10, 0, 5, light));
    // light bar left
    objects.add(make_shared<YZ_Rect>(-1, 1, -10, 0, -5, light));
    // light bar right
    objects.add(make_shared<YZ_Rect>(-1, 1, -10, 0, 5, light));
    objects.add(make_shared<XZ_Rect>(-0.5, 0.5, -10, 0, -5, light));

    // boxes
    objects.add(make_shared<Box>(Point3(-3, -5, -8), Point3(-1, 1, -6), metal));

    // mirror box
    shared_ptr<Hittable> mirrorBox = make_shared<Box>(Point3(-2, -5, -6), Point3(2, 3, -4), mirror);
    mirrorBox = make_shared<RotateY>(mirrorBox, 25);
    mirrorBox = make_shared<Translate>(mirrorBox, Vec3(4,0,-4));
    objects.add(mirrorBox);

    // glass sphere
    objects.add(make_shared<Sphere>(Point3( 0.0, 0.0, -2.0), 0.5, glass));
    objects.add(make_shared<Sphere>(Point3( -1, -1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( 1, -1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( -1, 1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( 1, 1, -3.0), 0.2, glass));
    return objects;
}

Hittable_List test_scene() {
    // background = Color(0.6,0.7,1);

    Hittable_List objects;

    auto perlinNoise = make_shared<Noise_Texture>(4);

    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Dielectric>(1.5);
    auto material_left   = make_shared<Metal>(Color(0.2, 0.4, 0.8), 0.2);
    auto material_right  = make_shared<Lambertian>(perlinNoise);
    auto light = make_shared<Diffuse_Light>(Color(1,1,1));

    objects.add(make_shared<XY_Rect>(-2, 2, 0, 2, -2, light));
    // objects.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // objects.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
    objects.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
    // objects.add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right)); 

    return objects;
}


int threads;
void renderScene(Camera& cam, Mat& image){

    threads = omp_get_max_threads();
    omp_set_num_threads(threads);

    for (int j = image_height-1; j >= 0; --j) {
        float progress = (float) (image_height - j)/ (float) image_height;
        displayProgressbar(progress);
        #pragma omp parallel for 
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            #pragma omp parallel for 
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
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
    std::cout << BOLDRED << "Welcome to my BasicRayTracer-Explorer!" << RESET << std::endl;
    std::cout << BOLDWHITE <<   "[W-A-S-D / arrow-keys to move and rotate]\n" <<
                                "[1-2 to move up and down]\n" <<
                                "[3-4 to tilt up and down]\n" <<
                                "[Q-E to zoom]\n" <<
                                "[O to save the current render as output.tiff]\n" <<
                                "[SPACE to render scene in high quality (720p)]\n" <<
                                "[ENTER to render scene in really high quality (1080p)]\n" <<
                                "[ESC to quit]\n" << RESET << std::endl;
}

void setLowQualityRender() {
    samples_per_pixel = 3;
    max_depth = 3;
    image_width = 400;
    image_height = 225;
}

void setHighQualityRender() {
    samples_per_pixel = 20;
    max_depth = 20;
    image_width = 1280;
    image_height = 720;
}

// warning! super high quality can take quite some time (multiple minutes, upto hours, depending on the scene) to compute :)
void setSuperHighQualityRender() {
    samples_per_pixel = 1000;
    max_depth = 50;
    image_width = 1920;
    image_height = 1080;
}

int main(int argc, char* argv[]) {
    // CXXOPTS Init and Parsing
    // Create CXXOPTS-Argument parser for nice argument input
    cxxopts::Options options("RayTracer", "A basic RayTracer, based on 'Ray Tracing In One Weekend' by Peter Shirley, adapted and extended by Martin Pluisch.");
    options.add_options()
        ("h,help", "Print usage information")
        ("x,width", "Image width in pixel", cxxopts::value<int>())
        ("y,height", "Image height in pixel", cxxopts::value<int>())
        ("s,samples_per_pixel", "Samples per pixel", cxxopts::value<int>())
        ("d,depth", "Maximum recursion depth for each ray", cxxopts::value<int>())
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
        // set background color
        

        world = modified_cornell();
    }

    // create image cv2-mat
    Mat image (image_height, image_width, CV_8UC3, Scalar(0,0,0));    
    // overwrite standard render-quality settings for faster first-frame rendering in explore mode
    if(explore) setLowQualityRender();
    // render initial scene-image
    
    #pragma parallel
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

            if (key == '3'){
                camDirection = camDirection - Vec3(0,exploreStepSize,0);
                cam.rotateCamera(camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == '4'){
                camDirection = camDirection + Vec3(0,exploreStepSize,0);
                cam.rotateCamera(camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            }



            if (key == 2||key == 'a'||key == 'A'){ // left arrow
                camDirection.rotateAroundPoint(currentCamPosition, -rotationAngle);
                cam.rotateCamera(camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 0||key == 'w'||key == 'W'){ // up arrow or uppper/lower-case w
                cam.moveCamera(camDirection * exploreStepSize, camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 3||key == 'd'||key == 'D'){ // right arrow
                camDirection.rotateAroundPoint(currentCamPosition, rotationAngle);
                cam.rotateCamera(camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            } else if (key == 1||key == 's'||key == 'S'){ // down arrow
                cam.moveCamera(camDirection * -exploreStepSize, camDirection, image_width, image_height, vfov, camAperture, camFocusDistance);
            }
            
            if (key == 32){ // spacebar -> render image in 1080p
                setHighQualityRender();
                resize(image, image, cv::Size(image_width, image_height));
                resizeWindow("RayTracer", image_width, image_height);
            } else if (key == 13){
                setSuperHighQualityRender();
                resize(image, image, cv::Size(image_width, image_height));
                resizeWindow("RayTracer", image_width, image_height);
            } 

            if (key == 'o'){
                imwrite("output.tiff",image);
            }
            
            
            if (key == 27){ // escape, end loop
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