cmake_minimum_required(VERSION 3.25)
project(Boids CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

include(FetchContent)

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

if(EMSCRIPTEN)
  set(PLATFORM Web CACHE STRING "" FORCE)
else()
  set(PLATFORM Desktop CACHE STRING "" FORCE)
endif()

FetchContent_Declare(
  raylib 
  GIT_REPOSITORY https://github.com/raysan5/raylib.git 
  GIT_TAG master)
FetchContent_MakeAvailable(raylib)

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

# Automatically include all header directories
file(GLOB_RECURSE HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
set(PROJECT_INCLUDE_DIRS "")
foreach(HEADER_FILE ${HEADER_DIRS})
    get_filename_component(HEADER_PATH ${HEADER_FILE} DIRECTORY)
    list(APPEND PROJECT_INCLUDE_DIRS ${HEADER_PATH})
endforeach()
list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)
include_directories(${PROJECT_INCLUDE_DIRS})

# Define the executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})

target_link_libraries(${PROJECT_NAME} raylib)

if(EMSCRIPTEN)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE 
    -msimd128 
    -msse4.1 
    -O3 
    -pthread 
    -std=c++20
)

# Linker flags
target_link_options(${PROJECT_NAME} PRIVATE -pthread)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".html"
    LINK_FLAGS "\
        -O3 \
        -lembind \
        -s USE_GLFW=3 \
        -s MAX_WEBGL_VERSION=2 \
        -s MODULARIZE=1 \
        -s EXPORT_ES6=1 \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s USE_PTHREADS=1 \
        -s PTHREAD_POOL_SIZE=16 \
        -s PTHREAD_POOL_SIZE_STRICT=2 \
        -s NO_DISABLE_EXCEPTION_CATCHING \
        --shell-file ${CMAKE_SOURCE_DIR}/src/Shell.html"
)
else()
  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
  else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx2 -msse2")
  set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS "-O3 -mavx2"
  )
  endif()
endif()
