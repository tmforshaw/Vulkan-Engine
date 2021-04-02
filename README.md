# Vulkan-Engine
An engine I am building to learn the vulkan API.

# Description
Originally I attempted to create an engine in OpenGL, however I want to try my hand at Vulkan because it is more powerful and modern.

# Preinstallation
You may find that when you try to compile the engine you are missing some files, you will need to run the following commands:
``` bash
sudo apt install build-essential
sudo apt install clang
```
This installs some basic C++ utilities, and the clang/clang++ compiler.

``` bash
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools
sudo apt install libglfw3-dev libglm-dev
```
This installs all of the necessary vulkan libraries, some tools to deal with compiled shaders, the GLFW framework, and the GLM library.

``` bash
sudo apt install libxxf86vm-dev
sudo apt install libxi-dev
```
This installs some libraries that are needed for the linker.
