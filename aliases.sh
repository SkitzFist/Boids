#!/bin/bash
alias build='cmake --build build_desktop & cmake --build build_web'
alias build_d='cmake --build build_desktop'
alias build_w='cmake --build build_web'
alias desktop='./build_desktop/Boids'
alias web='emrun ./build_web/Boids.html'
alias p_d='sudo perf stat -B -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations ./build_desktop/Boids'
