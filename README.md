# Boids
A simple boid simulation


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
