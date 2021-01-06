#include <math.h>
#include <omp.h>                // used for parallel/multi-processing
#include <iostream>
#include <cxxopts.hpp>          // used for nicer argument parsing
#include <opencv2/opencv.hpp>   // used for displaying rendered frame directly and moving around the scenery


#include "Utilities.h"
#include "Color.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"
#include "AARect.h"
#include "Box.h"
#include "Constant_Medium.h"

using namespace cv;

// --- VARIABLES ---
#pragma region Variables
// -- Camera vars --
Camera cam;
int image_width;
int image_height;
int samples_per_pixel;
int max_depth;
double vfov;
// -- Scene vars --
Hittable_List world;
Color background(0,0,0);
int spawnObjectDistance = 1;
int currentSceneIndex = 0;
int numOfDifferentScenes = 2;

// -- Exploration vars --
// rotation angle in degree per step (= key-press)
float rotationAngle = 10;
// units to move per arrow-keypress
float exploreStepSize = 0.1;
// units to increase/decrease fov per step
float vfovStep = 1;

#pragma endregion
// -----------------


// --- RAYTRACING + RENDERING ---
#pragma region RayTracing and Rendering
/// helper-struct for more efficient mat-pixel settings (per https://stackoverflow.com/questions/23001512/c-and-opencv-get-and-set-pixel-color-to-mat)
struct RGB {uchar blue; uchar green; uchar red;};

/// Calculates and returns a ray's color in a given scene, for a given maximum recursion-depth
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

/// Renders the current world-scene onto a passed-in OpenCV Mat using a passed-in camera object
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

#pragma endregion
// ------------------------------


// --- CAMERA INIT ---
#pragma region Camera Init
/// Initializes the camera object for the current runtime
void initializeCamera(Camera &cam) {
    Point3 currentCamPosition (0,0,0);
    Point3 camDirection = currentCamPosition + Vec3(0,0,-1);
    Vec3 camUp (0,1,0);
    double camAperture = 0.0;
    double camFocusDistance = (currentCamPosition-camDirection).length();
    Camera c(currentCamPosition, camDirection, camUp, image_width, image_height, vfov, camAperture, camFocusDistance);
    cam = c;
}
#pragma endregion
// -------------------


// --- PRESET SCENE DEFINITION ---
#pragma region Preset Scene Definition
/// returns my version of a "cornell-box"
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

    // wall left
    objects.add(make_shared<YZ_Rect>(-5, 5, -10, 0, -5, green));
    // wall right
    objects.add(make_shared<YZ_Rect>(-5, 5, -10, 0, 5, yellow));
    // ceiling
    objects.add(make_shared<XZ_Rect>(-5, 5, -10, 0, 5, red));
    // ground
    objects.add(make_shared<XZ_Rect>(-5, 5, -10, 0, -5, red));
    // wall back
    objects.add(make_shared<XY_Rect>(-5, 5, -5, 5, -10, white));
    // wall front
    objects.add(make_shared<XY_Rect>(-5, 5, -5, 5, 0.1, white));

    // light-bar top
    objects.add(make_shared<XZ_Rect>(-0.5, 0.5, -10, 0, 5, light));
    // light-bar left
    objects.add(make_shared<YZ_Rect>(-1, 1, -10, 0, -5, light));
    // light-bar right
    objects.add(make_shared<YZ_Rect>(-1, 1, -10, 0, 5, light));
    // light-bar ground
    objects.add(make_shared<XZ_Rect>(-0.5, 0.5, -10, 0, -5, light));

    // metal box
    objects.add(make_shared<Box>(Point3(-3, -5, -8), Point3(-1, 1, -6), metal));

    // mirror box
    shared_ptr<Hittable> mirrorBox = make_shared<Box>(Point3(-2, -5, -6), Point3(2, 3, -4), mirror);
    mirrorBox = make_shared<RotateY>(mirrorBox, 25);
    mirrorBox = make_shared<Translate>(mirrorBox, Vec3(4,0,-4));
    objects.add(mirrorBox);

    // glass spheres
    objects.add(make_shared<Sphere>(Point3( 0.0, 0.0, -2.0), 0.5, glass));       
    objects.add(make_shared<Sphere>(Point3( -1, -1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( 1, -1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( -1, 1, -3.0), 0.2, glass));
    objects.add(make_shared<Sphere>(Point3( 1, 1, -3.0), 0.2, glass));

    return objects;
}

/// returns a test-scene with (a close-enough :)) H-BRS-Logo
Hittable_List test_scene() {
    background = Color(0.9,0.75,0.5);
    Hittable_List objects;

    auto light = make_shared<Diffuse_Light>(Color(1,1,1));
    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));                        
    auto hbrs_mat = make_shared<Lambertian>(Color(0.28, 0.6, 0.98));
    
    // light bar
    objects.add(make_shared<XY_Rect>(-2, 2, 4, 6, 2, light));
    // left sphere
    objects.add(make_shared<Sphere>(Point3(-0.55,    0.0, -1.0),   0.5, hbrs_mat));
    // "white" inner sphere, use light instead of white for nicer visuals
    objects.add(make_shared<Sphere>(Point3(-0.55,    0.0, -0.725),   0.3, light));
    // right sphere
    objects.add(make_shared<Sphere>(Point3(0.55,    0.0, -1.0),   0.5, hbrs_mat));

    return objects;
}

/// returns an empty, well-illuminated "playground"
Hittable_List playground() {
    // set to "night time" for better light-fx
    background = Color(0,0,0);
    Hittable_List objects;

    auto checker_ground = make_shared<Lambertian>(make_shared<Checker_Texture>(Color(0.5, 0.7, 0.3), Color(0.9, 0.9, 0.9)));
    auto light = make_shared<Diffuse_Light>(Color(1,1,1));

    // ground
    objects.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, checker_ground));
    // light bars at the back
    objects.add(make_shared<XY_Rect>(-10, 10, 7, 9, -10, light));
    objects.add(make_shared<XY_Rect>(-11, 11, 4, 5, -11, light));
    objects.add(make_shared<XY_Rect>(-12, 12, 1.5, 2, -12, light));
    // light bars at the front
    objects.add(make_shared<XY_Rect>(-10, 10, 7, 9, 10, light));
    objects.add(make_shared<XY_Rect>(-11, 11, 4, 5, 11, light));
    objects.add(make_shared<XY_Rect>(-12, 12, 1.5, 2, 12, light));
    // light bars to the left
    objects.add(make_shared<YZ_Rect>(7, 9, -10, 10, -10, light));
    objects.add(make_shared<YZ_Rect>(4, 5, -11, 11, -11, light));
    objects.add(make_shared<YZ_Rect>(1.5, 2, -12, 12, -12, light));
    // light bars to the right
    objects.add(make_shared<YZ_Rect>(7, 9, -10, 10, 10, light));
    objects.add(make_shared<YZ_Rect>(4, 5, -11, 11, 11, light));
    objects.add(make_shared<YZ_Rect>(1.5, 2, -12, 12, 12, light));

    return objects;
}
#pragma endregion
// -------------------------------


// --- RENDER-QUALITY ---
#pragma region Render Quality Setters
/// Sets Low Quality settings used as standard settings for the explorer-mode.
void setLowQualityRender() {
    samples_per_pixel = 3;
    max_depth = 3;
    image_width = 400;
    image_height = 225;
}
/// Sets 720p - High Quality render settings for pretty fast rendering with a decent output.
void setHighQualityRender() {
    samples_per_pixel = 20;
    max_depth = 20;
    image_width = 1280;
    image_height = 720;
}

/// Sets 1080p - Super High Quality render settings for very slow rendering but really nice output.
/// Warning! Super High Quality can take quite some time (multiple minutes, depending on the scene) to compute :)
void setSuperHighQualityRender() {
    samples_per_pixel = 1000;
    max_depth = 50;
    image_width = 1920;
    image_height = 1080;
}
#pragma endregion
// ----------------------


// --- OBJECT GENERATION ---
#pragma region Object Generation
/// Generates and adds a sphere of random size at a specified position with a specified material
void spawnSphere(Point3 spawnPoint, int material) {
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

/// Generates and adds a box of random size at a specified position with a specified material
void spawnBox(Point3 spawnPoint, int material) {
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
#pragma endregion
// -------------------------


// --- MISC HELPER FUNCTIONS ---
#pragma region Misc. Helper Functions
/// Increases current scene-index (used for switching between scenes) and takes care of value-wrap-around
void increaseSceneIndex(){
    currentSceneIndex++;
    if(currentSceneIndex > numOfDifferentScenes){
        currentSceneIndex = 0;
    }
}

/// Loads scene specified by current scene-index, by (re-)setting the world variable
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

/// Prints a short intro-/tutorial-message for the Explorer-feature
void printExploreWelcomeMessage(){
    // show short intro message
    std::cout << BOLDRED << "Welcome to my RayTrace-Playground!\n" << RESET << std::endl;
    std::cout << BOLDWHITE <<   "[W-A-S-D to move and rotate]\n" <<
                                "[1-2 to move up and down]\n" <<
                                "[3-4 to tilt up and down]\n" <<
                                "[Q-E to zoom]\n" <<
                                "[R-T to modify focus distance]\n" <<
                                "[Z-U to modify aperture]\n" <<
                                "[8-9-0 to set the sky to night-day-random color]\n" <<
                                "[TAB to change the current scene]\n\n" <<

                                "[G-H-J to spawn a diffuse-metallic-glass sphere]\n" <<
                                "[B-N-M to spawn a diffuse-metallic-glass box]\n\n" <<

                                "[SPACE to render scene in high quality (720p)]\n" <<
                                "[ENTER to render scene in really high quality (1080p)]\n" <<
                                "[O to save the current render as .tiff in ../generated_images/]\n" <<
                                "[ESC to quit]\n" << RESET << std::endl;
}

#pragma endregion
// -----------------------------


int main(int argc, char* argv[]) {
    #pragma region CXXOPTS Init
    // CXXOPTS Init
    // Create CXXOPTS-Argument parser for nice argument input
    cxxopts::Options options("RayTrace-Playground", "A basic RayTracer, based on 'Ray Tracing In One Weekend' and 'Ray Tracing The Next Week', adapted and extended by Martin Pluisch.");
    options.add_options()
        ("h,help",                  "Print usage information")
        ("x,width",                 "Image width in pixel",                         cxxopts::value<int>())
        ("y,height",                "Image height in pixel",                        cxxopts::value<int>())
        ("s,samples_per_pixel",     "Samples per pixel",                            cxxopts::value<int>())
        ("d,depth",                 "Maximum recursion depth for each ray",         cxxopts::value<int>())
        ("f,fov",                   "Camera's vertical field of view",              cxxopts::value<double>())
        ("o,output",                "Output file name, without file extension",     cxxopts::value<std::string>())

        ("e,explore",               "Start explorer mode",                          cxxopts::value<bool>())
    ;
    #pragma endregion

    #pragma region CXXOPTS Parsing
    // -- CXXOPTS Parsing --
    auto result = options.parse(argc, argv);

    if (result.count("help") == 1) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    // Read and store passed in arguments or default to 400x225 (16:9) resolution with 100 samples per pixel
    image_width         = (result.count("width") ? result["width"].as<int>() : 400);
    image_height        = (result.count("height") ? result["height"].as<int>() : 225);
    samples_per_pixel   = (result.count("samples_per_pixel") ? result["samples_per_pixel"].as<int>() : 50);
    max_depth           = (result.count("depth") ? result["depth"].as<int>() : 50);
    vfov                = (result.count("fov") ? result["fov"].as<double>() : 90);
    std::string output  = (result.count("output") ? result["output"].as<std::string>() : "output");
    bool explore        = (result.count("explore") ? result["explore"].as<bool>() : false);
    #pragma endregion
    
    if(explore){
        // if user enters explore-mode, print quick tutorial/welcome-message
        printExploreWelcomeMessage();
    } else {
        // otherwise set scene-to-render to my modified "cornell"-box
        currentSceneIndex = 1;
    }

    // Generate Camera
    initializeCamera(cam);
    // Initialize World, using standard index 0       
    loadSceneByIndex();    
    // create image cv2-mat
    Mat image (image_height, image_width, CV_8UC3, Scalar(0,0,0));
    // overwrite standard render-quality settings for faster first-frame rendering in explore mode
    if(explore) setLowQualityRender();    
    // render initial scene-image
    renderScene(cam, image);
    // create OpenCV window
    namedWindow("RayTrace-Playground", WINDOW_AUTOSIZE);

    #pragma region World-Exploring through movement, displayed in OpenCV window
    if(explore){        
        while(explore){
            // (re-)set low-quality setting for faster rendering and thus allow "real-time" exploration
            setLowQualityRender();      
            
            // show rendered image using opencv
            imshow("RayTrace-Playground", image);

            // read input-keys
            char key = cv::waitKey(0);

            // ===== INTERPRET INPUT KEYS =====
            #pragma region Object-Generation
            // --- SPHERE GENERATION + SPAWNING ---
            // diffuse sphere
            if (key == 'g'||key == 'G'){
                spawnSphere(cam.getCurrentDirection(), 0);
            }
            // metallic sphere
            else if (key == 'h'||key == 'H'){
                spawnSphere(cam.getCurrentDirection(), 1);
            }
            // glass sphere
            else if (key == 'j'||key == 'J'){
                spawnSphere(cam.getCurrentDirection(), 2);
            }
            // ------------------------------------
            
            // --- BOX GENERATION + SPAWNING ---
            // diffuse box
            if (key == 'b'||key == 'B'){
                spawnBox(cam.getCurrentDirection(), 0);
            } 
            // metallic box
            else if (key == 'n'||key == 'N'){
                spawnBox(cam.getCurrentDirection(), 1);
            }
            // glass box
            else if (key == 'm'||key == 'M'){
                spawnBox(cam.getCurrentDirection(), 2);
            }
            // ---------------------------------
            #pragma endregion

            #pragma region Camera-Manipulation
            // --- CAMERA MOVEMENT ---
            // forward movement
            if (key == 'w'||key == 'W'){
                cam.moveCamera(cam.getRelativeViewDirection() * exploreStepSize);
            } 
            // backwards movement
            else if (key == 's'||key == 'S'){
                cam.moveCamera(cam.getRelativeViewDirection() * -exploreStepSize);
            }
            // downwards movement
            else if(key == '1'){
                cam.moveCamera(Vec3(0,-exploreStepSize,0));
            } 
            // upwards movement
            else if(key == '2'){
                cam.moveCamera(Vec3(0,exploreStepSize,0));
            }
            // -----------------------


            // --- CAMERA ROTATION ---
            // left-rotation
            if (key == 'a'){
                cam.rotateCamera(-rotationAngle);
            } 
            // right-rotation
            else if (key == 'd'){
                cam.rotateCamera(rotationAngle);
            } 
            // tilt downwards
            else if (key == '3'){
                cam.tiltCamera(Vec3(0, -exploreStepSize, 0));
            } 
            // tilt upwards
            else if (key == '4'){
                cam.tiltCamera(Vec3(0, exploreStepSize, 0));
            }
            // -----------------------


            // --- FOCUS DISTANCE ---
            // focusDistance decrease
            if (key == 'r'){
                cam.updateFocusDistance(-exploreStepSize);
            } 
            // focusDistance increase
            else if (key == 't'){
                cam.updateFocusDistance(exploreStepSize);
            }
            // ----------------------


            // --- APERTURE ---
            // Aperture decrease
            if (key == 'z'){
                cam.updateAperture(-exploreStepSize);
            } 
            // Aperture increase
            else if (key == 'u'){
                cam.updateAperture(exploreStepSize);
            }
            // ----------------


            // --- FIELD OF VIEW ---
            // decrease fov
            if(key == 'e'){
                cam.updateFOV(-vfovStep);
            }
            // increase fov
            else if (key == 'q'){
                cam.updateFOV(vfovStep);
            }
            // ---------------------
            #pragma endregion

            #pragma region Full-Rendering
            // --- FULL RENDERING ---
            // 720p, high quality render, press SPACE
            if (key == 32){ 
                setHighQualityRender();
                resize(image, image, cv::Size(image_width, image_height));
                resizeWindow("RayTracer", image_width, image_height);
            }
            // 1080p, very high quality render, press ENTER
            else if (key == 13){
                setSuperHighQualityRender();
                resize(image, image, cv::Size(image_width, image_height));
                resizeWindow("RayTracer", image_width, image_height);
            } 
            // ----------------------
            #pragma endregion

            #pragma region Misc. Tools/Manipulations
            // --- BACKGROUND/SKYBOX SWITCH ---
            // pitch-black sky
            if(key == '8'){
                background = Color(0,0,0);
            } 
            // light blue, daylight sky
            else if (key == '9'){
                background = Color(0.5, 0.7, 1);
            }
            // random color
            else if (key == '0'){
                background = Color::random();
            }
            // --------------------------------


            // --- SCENE SWITCHER ---
            // TAB-key
            if (key == 9){  
                // safely increase (and wrap-around) scene index
                increaseSceneIndex();
                loadSceneByIndex();
                // re-initialize camera to reset position, rotation, tilt...
                initializeCamera(cam);
            }
            // ----------------------

            
            // --- GENERAL ---
            // Save output image
            if (key == 'o'){
                std::string filename = random_string(10);
                imwrite("../generated_images/" + filename + ".tiff",image);
            } 
            // ESC, end program
            else if (key == 27){ 
                explore = false;
            } 
            // ---------------
            #pragma endregion
            // ================================
            
            else renderScene(cam, image);
        }   
    #pragma endregion
    } else {
        // show and save render
        imshow("RayTrace-Playground", image);
        imwrite("../generated_images/" + output + ".tiff",image);    
        cv::waitKey(0);
    }
}
