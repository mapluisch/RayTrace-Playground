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
#include "Constant_Medium.h"

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
int spawnObjectDistance = 1;
Camera cam;

int currentSceneIndex = 0;
int numOfDifferentScenes = 2;

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

Hittable_List modified_cornell() {
    Hittable_List objects;
    background = Color(0,0,0);

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
    background = Color(0,0,0.2);
    Hittable_List objects;

    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto sphere_mat = make_shared<Dielectric>(1.5);
    auto light = make_shared<Diffuse_Light>(Color(1,1,1));

    objects.add(make_shared<XY_Rect>(-2, 2, 0, 2, -2, light));
    objects.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    objects.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, sphere_mat));

    return objects;
}

Hittable_List playground() {
    background = Color(0,0,0);
    Hittable_List objects;

    auto checker_ground = make_shared<Lambertian>(make_shared<Checker_Texture>(Color(0.5, 0.7, 0.3), Color(0.9, 0.9, 0.9)));
    auto light = make_shared<Diffuse_Light>(Color(1,1,1));

    objects.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, checker_ground));
    objects.add(make_shared<XY_Rect>(-10, 10, 7, 9, -10, light));
    objects.add(make_shared<XY_Rect>(-11, 11, 4, 5, -11, light));
    objects.add(make_shared<XY_Rect>(-12, 12, 1.5, 2, -12, light));

    objects.add(make_shared<XY_Rect>(-10, 10, 7, 9, 10, light));
    objects.add(make_shared<XY_Rect>(-11, 11, 4, 5, 11, light));
    objects.add(make_shared<XY_Rect>(-12, 12, 1.5, 2, 12, light));

    objects.add(make_shared<YZ_Rect>(7, 9, -10, 10, -10, light));
    objects.add(make_shared<YZ_Rect>(4, 5, -11, 11, -11, light));
    objects.add(make_shared<YZ_Rect>(1.5, 2, -12, 12, -12, light));

    objects.add(make_shared<YZ_Rect>(7, 9, -10, 10, 10, light));
    objects.add(make_shared<YZ_Rect>(4, 5, -11, 11, 11, light));
    objects.add(make_shared<YZ_Rect>(1.5, 2, -12, 12, 12, light));


    return objects;
}

void increaseSceneIndex(){
    currentSceneIndex++;
    if(currentSceneIndex > numOfDifferentScenes){
        currentSceneIndex = 0;
    }
}

void loadSceneByIndex(){
    switch(currentSceneIndex){
        case 0:
            world = playground();
            break;
        case 1:
            world = modified_cornell();
            break;
        case 2:
            world = test_scene();
            break;
    }
}

void renderScene(Camera& cam, Mat& image){

    int threads = omp_get_max_threads();
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
    std::cout << BOLDRED << "Welcome to my BasicRayTracer-Explorer!\n" << RESET << std::endl;
    std::cout << BOLDWHITE <<   "[W-A-S-D / arrow-keys to move and rotate]\n" <<
                                "[1-2 to move up and down]\n" <<
                                "[3-4 to tilt up and down]\n" <<
                                "[Q-E to zoom]\n" <<
                                "[8-9-0 to set the sky to night-day-random color]\n" <<
                                "[TAB to change the current scene]\n\n" <<

                                "[G-H-J to spawn a diffuse-metallic-glass sphere]\n" <<
                                "[B-N-M to spawn a diffuse-metallic-glass box]\n\n" <<

                                "[SPACE to render scene in high quality (720p)]\n" <<
                                "[ENTER to render scene in really high quality (1080p)]\n" <<
                                "[O to save the current render as .tiff in ../generated_images/]\n" <<
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

void spawnRandomSphere(Point3 spawnPoint, int material) {
    auto random_material = random_double();
    float min_size= 0.25;
    float max_size = 0.75;
    auto random_size = random_double(min_size, max_size);
    Point3 center = spawnPoint + (cam.getRelativeViewDirection() * spawnObjectDistance);

    shared_ptr<Material> sphere_material;

    if (material == 0) {
        // diffuse
        auto albedo = Color::random();
        sphere_material = make_shared<Lambertian>(albedo);
    } else if (material == 1) {
        // metal
        auto albedo = Color::random();
        auto fuzz = random_double(0, 0.5);
        sphere_material = make_shared<Metal>(albedo, fuzz);
    } else {
        // glass
        float minRefraction = 1.2;
        sphere_material = make_shared<Dielectric>(random_double() + minRefraction);
    }   
    world.add(make_shared<Sphere>(center, random_size, sphere_material));
}

void spawnRandomBox(Point3 spawnPoint, int material) {
    auto random_material = random_double();
    float min_size= 0.25;
    float max_size = 0.75;
    auto random_size = random_double(min_size, max_size);

    Point3 center = spawnPoint + (cam.getRelativeViewDirection() * spawnObjectDistance);

    shared_ptr<Material> box_material;

    if (material == 0) {
        // diffuse
        auto albedo = Color::random();
        box_material = make_shared<Lambertian>(albedo);
    } else if (material == 1) {
        // metal
        auto albedo = Color::random();
        auto fuzz = random_double(0, 0.5);
        box_material = make_shared<Metal>(albedo, fuzz);
    } else {
        // glass
        float minRefraction = 1.2;
        box_material = make_shared<Dielectric>(random_double() + minRefraction);
    }   
    world.add(make_shared<Box>(spawnPoint - Vec3(random_size/2), spawnPoint + Vec3(random_size/2), box_material));
}

void InitializeCamera(Camera &cam) {
    Point3 currentCamPosition (0,0,0);
    Point3 camDirection = currentCamPosition + Vec3(0,0,-1);
    Vec3 camUp (0,1,0);
    double camAperture = 0.0;
    double camFocusDistance = (currentCamPosition-camDirection).length();
    Camera c(currentCamPosition, camDirection, camUp, image_width, image_height, vfov, camAperture, camFocusDistance);
    cam = c;
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
    InitializeCamera(cam);

    // Initialize World, using standard index 0       
    loadSceneByIndex();
    

    // create image cv2-mat
    Mat image (image_height, image_width, CV_8UC3, Scalar(0,0,0));    
    // overwrite standard render-quality settings for faster first-frame rendering in explore mode
    if(explore) setLowQualityRender();
    // render initial scene-image
    
    renderScene(cam, image);

    namedWindow("RayTracer", WINDOW_AUTOSIZE);

    #pragma region World-Exploring through movement, displayed in OpenCV window
    if(explore){        
        while(explore){
            // set low-quality setting for faster rendering
            setLowQualityRender();      
            
            imshow("RayTracer", image);            
            
            char key = cv::waitKey(0);

            if(key == 'e'){
                cam.updateFOV(-vfovStep);
            } else if (key == 'q'){
                cam.updateFOV(vfovStep);
            }

            if(key == '1'){
                cam.moveCamera(Vec3(0,-exploreStepSize,0));
            } else if(key == '2'){
                cam.moveCamera(Vec3(0,exploreStepSize,0));
            }

            if (key == '3'){
                cam.tiltCamera(Vec3(0, -exploreStepSize, 0));
            } else if (key == '4'){
                cam.tiltCamera(Vec3(0, exploreStepSize, 0));
            }

            if (key == 'g'||key == 'G'){
                spawnRandomSphere(cam.getCurrentDirection(), 0);
            } else if (key == 'h'||key == 'H'){
                spawnRandomSphere(cam.getCurrentDirection(), 1);
            } else if (key == 'j'||key == 'J'){
                spawnRandomSphere(cam.getCurrentDirection(), 2);
            } 
            
            if (key == 'b'||key == 'B'){
                spawnRandomBox(cam.getCurrentDirection(), 0);
            } else if (key == 'n'||key == 'N'){
                spawnRandomBox(cam.getCurrentDirection(), 1);
            } else if (key == 'm'||key == 'M'){
                spawnRandomBox(cam.getCurrentDirection(), 2);
            }

            if(key == '8'){
                background = Color(0,0,0);
            } else if (key == '9'){
                background = Color(0.5, 0.7, 1);
            } else if (key == '0'){
                background = Color::random();
            }

            if (key == 2||key == 'a'||key == 'A'){ // left movement
                cam.rotateCamera(-rotationAngle);
            } else if (key == 0||key == 'w'||key == 'W'){ // up movement
                cam.moveCamera(cam.getRelativeViewDirection() * exploreStepSize);
            } else if (key == 3||key == 'd'||key == 'D'){ // right movement
                cam.rotateCamera(rotationAngle);
            } else if (key == 1||key == 's'||key == 'S'){ // down movement
                cam.moveCamera(cam.getRelativeViewDirection() * -exploreStepSize);
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


            if (key == 9){ // tab, switch through scenes
                // safely increase (and wrap-around) scene index
                increaseSceneIndex();
                loadSceneByIndex();
                // re-initialize camera to reset position, rotation, tilt...
                InitializeCamera(cam);
            }

            if (key == 'o'){
                std::string filename = random_string(10);
                imwrite("../generated_images/" + filename + ".tiff",image);
            } else if (key == 27){ // escape, end loop
                explore = false;
            } else renderScene(cam, image);
        }   
    } else {
        namedWindow("RayTracer", WINDOW_AUTOSIZE);// Create a window for display.
        imshow("RayTracer", image);    
        cv::waitKey(0);
    }
    #pragma endregion
}