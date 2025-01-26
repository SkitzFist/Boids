# Boids
A simple boid simulation. It's a fun challenge on how far I can push the CPU.

Goal: to run 10 million boids, where every single boid will get updated every single frame. (not necesarilly rendered though)


# Compilation
Compiles for desktop and web with CMake.

Just run cmake with your preffered compiler, no game related flags is needed.

example
```
//linux
cmake -S . -B build

//web
emcmake cmake -S . -B build_web
```

# Notes
As this is still under development, AVX2 capabilities is needed for desktop.
When finished it should be fine to run with only SSE 4.1 capabilities. 

Reccomended to have at least 6 physical cores. 

The project "should" run on windows, but has not been tested there yet.

# Linux dependencies

Wayland dependencies:

    libwayland-dev
    libxkbcommon-dev

X11 dependencies (important for graphics and windowing support):

    libx11-dev
    libxcursor-dev
    libxrandr-dev
    libxi-dev
    libgl1-mesa-dev
    libasound2-dev 
