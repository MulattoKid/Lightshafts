# Lightshafts
This is what I created as my final project in TDT4230 (Graphics and Visualization) at NTNU.

The work of (Alexandre Pestana)[http://www.alexandre-pestana.com/volumetric-lights/] and (Benjamin Glatzel)[https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen] were of huge help as they gave helped me understand the basic concept as well some neat techniques for improving performance without losing too much quality.

![](https://i.imgur.com/SkOWMLh.png)

## Current rendering steps
- Shadow map creation [1024x1024]
- GBuffer (depth, world position and color lit by one light) [1920x1080]
- Compute scattering (lightshaft) [960x540]
  - One ray per pixel
  - 32 samples per pixel (stop at depth stored in GBuffer)
  - Use Bayer-Matrix to make up for few samples
- Add scattering [1920x1080]
- Particles (instanced rendering) [1920x1080]

## Code
All rendering code is located in *window.cpp*. Data used in multiple shaders is located in a shared UBO, while texture and individual uniforms have their own uniform variable per shader. The most interesting shader is *compute_scattering.frag*. I think the code is prett easy-to-read (I recommend referring to the two above links for reference). I have tried commenting where I feel the code is not always self-explanatory, but I might have missed some places.

## Performance
I have mainly ran this on my laptop which has the following noteworthy specs:
- Intel Core i7-4710HQ @ 2.5GHz (4 cores, 8 logical processors)
- Nvidia Gefore GTX 850M @ ~930MHz (640 cores)
When running with optimizations (Release in Visual Studio), I achieved the following results when having ~16000 particles in the scene:
- Frame time:14-15ms
  - Particle update time: 4ms
  - Shadow map time: 1-2ms
  - GBuffer time: 5ms
  - Lightshaft time: 1-2ms
  - Particle render time: 0-1ms
The current scene has statue which has quite a lot of triangles (~448K), which is why the GBuffer pass takes as long as it does.
### NOTE: the program should be run with O2 optimizations turned on, otherwise, the loading of the .obj file will take a long time

## CMake
The code has four dependencies:
- OpenGL *(minimum version 4.3)*
- GLEW
- SDL2
- OpenMP
### Linux
I do not have a Linux machine to test the program on myself, but the CMake should create and copy the necessary files to compile. If you have troubles with some of the assets, I would recommend to just compy them directly into the folder where the executable is generated (most likely the directory from which you ran CMake).
