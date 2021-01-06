
# RayTrace-Playground

![ModifiedCornell](generated_images/modified_cornell.tiff)

Basic RayTracer written in C++, based on the books "Ray Tracing in One Weekend" and "Ray Tracing The Next Week" by Peter Shirley, adapted and extended by me.

Instead of "simply" rendering and saving an output image, I use OpenCV to directly display the rendered output plus read the user's input keys to move around the scene, manipulate the camera and spawn objects.

I've also included OpenMP's parallel-directives to speed up the ray tracing process.

## Requirements
- OpenCV 
- CXXOPTS
- OpenMP

## Build
Simply `cd` into the build folder and build via `cmake ..`

Be aware of the special OpenMP Link- and Include-Directories that I've had to set to make OpenMP run under macOS Big Sur - if you compile my program using a different macOS version / Windows / Linux, remove those entries beforehand.

## Playground
To explore my RayTrace-Playground, simply call

`./RayTrace-Playground -e`

### Controls
You'll get a console output with all control
```
[W-A-S-D to move and rotate]
[1-2 to move up and down]
[3-4 to tilt up and down]
[Q-E to zoom]
[R-T to modify focus distance]
[Z-U to modify aperture]

[8-9-0 to set the sky to night-day-random color]
[TAB to change the current scene]

[G-H-J to spawn a diffuse-metallic-glass sphere]
[B-N-M to spawn a diffuse-metallic-glass box]

[SPACE to render scene in high quality (720p)]
[ENTER to render scene in really high quality (1080p)]

[O to save the current render as .tiff in ../generated_images/]
[ESC to quit]
```
## Arguments
If you'd only like to render my modified cornell-box, you can just call
```
./RayTrace-Playground
```
with a combination of the following arguments, used for static scene rendering:
```
h,help 					Print usage information
x,width 				Image width in pixel
y,height 				Image height in pixel
s,samples_per_pixel 	  		Samples per pixel
d,depth 				Maximum recursion depth for each ray
f,fov 					Camera's vertical field of view
o,output 				Output file name, without file extension
```
Again, to explore the playground:
```
e,explore 				Start explorer mode
```
## Samples
![1](generated_images/1.tiff)
![2](generated_images/2.tiff)
![3](generated_images/3.tiff)
