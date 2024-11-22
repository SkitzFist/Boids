#include "ViewPort.h"

#include "raylib.h"

#include "Log.h"

void createViewPort(bool isWeb) {
    int screenWidth = 1280;
    int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Boids");
}