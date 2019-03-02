# bp
Boilerplate (bp) abstraction library for Vulkan API, implemented in C++. Aims to reduce the amount of code needed to implement Vulkan based applications, while still being flexible.

This library was implemented as part of my master thesis in software development, ["Multi-GPU Rendering with Vulkan API"](https://www.dropbox.com/s/vcxopmghzojahb1/lot-mgpu-rendering-vulkan.pdf?dl=1). A paper on the subject has been submitted to [Norwegian Informatics Conference (NIK)](http://ojs.bibsys.no/index.php/NIK/article/view/513).
## Modules
* **bp** is the core module containing the Vulkan abstractions.
* **bpMulti** is the module providing multi-GPU abstractions for the sort-first and sort-last approach (renderers for partial results, and compositors for combining the results).
* **bpQt** provides a Qt window that should be used as a render target by **bp**.
* **bpScene** provides a way to load meshes and materials, render them, and transform the position and orientation with a simple scene graph structure.
* **bpView** is a wrapper of GLFW to use a GLFW window as a render target.
