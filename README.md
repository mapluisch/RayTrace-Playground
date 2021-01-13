
# RayTrace-Playground

![My modified version of a cornell box](generated_images/modified_cornell.tiff)

Basic RayTracer written in C++, based on the books "Ray Tracing in One Weekend" and "Ray Tracing The Next Week" by Peter Shirley (which can be found [here on GitHub](https://github.com/RayTracing/raytracing.github.io)).

Instead of rendering a given scene directly into an output file, I use OpenCV to display the rendered output plus read the user's input keys to move around the scene, manipulate the camera and spawn objects.

I've also included [OpenMP's](https://www.openmp.org) parallel-directives to speed up the ray tracing process and added some useful input-arguments, which are parsed via [CXXOPTS](https://github.com/jarro2783/cxxopts).

## Requirements
- [OpenCV](https://github.com/opencv/opencv)
- [CXXOPTS](https://github.com/jarro2783/cxxopts)
- [OpenMP](https://www.openmp.org)

## Build
Simply `cd` into the build folder and build via `cmake ..`

**Important:** Be aware of the special OpenMP Link- and Include-Directories that I've had to set to make OpenMP run under macOS Big Sur - if you compile my program using a different macOS version / Windows / Linux, remove those entries beforehand.

## Playground
To explore my RayTrace-Playground, simply call

`./RayTrace-Playground -e`

### Controls
You'll get a console output with all possible inputs, as soon as you start the explore-mode.
Regardless, here's a quick overview:
```
[W-A-S-D to move and rotate]
[1-2 to move up and down]
[3-4 to tilt up and down]
[Q-E to zoom]
[R-T to modify focus distance]
[Z-U to modify aperture]

[8-9-0 to set the sky to night-day-random color]
[TAB to change the current scene]

[G-H-J-K to spawn a diffuse-metallic-glass-light sphere]
[V-B-N-M to spawn a diffuse-metallic-glass-light box]

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
![2](generated_images/2.tiff)
![6](generated_images/6.tiff)
![7](generated_images/7.tiff)
![8](generated_images/8.tiff)
