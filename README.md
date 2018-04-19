# Lightshafts
This is what I created as my fianl project in TDT4230 (Graphics and Visualization) at NTNU.

The work of (Alexandre Pestana)[http://www.alexandre-pestana.com/volumetric-lights/] and (Benjamin Glatzel)[https://www.slideshare.net/BenjaminGlatzel/volumetric-lighting-for-many-lights-in-lords-of-the-fallen] were of huge help as they gave helped me understand the basic concept as well some neat techniques for improving performance without losing too much quality.

## Current rendering steps
- Shadow map creation [1024x1024]
- GBuffer (depth, world position and color lit by one light) [1920x1080]
- Compute scattering (lightshaft) [960x540]
  - One ray per pixel
  - 32 samples per pixel (stop at depth stored in GBuffer)
  - Use Bayer-Matrix to make up for few samples
- Add scattering [1920x1080]
- Particles (instanced rendered) [1920x1080]

## Code
All rendering code is located in *window.cpp*. Data used in multiple shaders is located in a shared UBO. There are currently ~16000 particles in the scene, each with its own model matrix which is used to update it at the start of each frame.
