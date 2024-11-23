#include "ViewPort.h"

#include "raylib.h"

#include "Log.h"

void createViewPort(bool isWeb) {
    int screenWidth = 1920;
    int screenHeight = 1080;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Boids");
}