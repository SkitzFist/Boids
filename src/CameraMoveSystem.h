#ifndef BOIDS_CAMERA_MOVE_SYSTEM_H
#define BOIDS_CAMERA_MOVE_SYSTEM_H

#include "raylib.h"

void updateCamera(Camera2D& camera, const float dt);
void handleMove(Camera2D& camera, const float dt);
void handleZoom(Camera2D& camera, const float dt);
void moveTowardsMouse(Camera2D& camera);

#endif
