#include "CameraMoveSystem.h"

void updateCamera(Camera2D& camera, const float dt) {
    handleZoom(camera, dt);
    handleMove(camera, dt);
}

void handleMove(Camera2D& camera, const float dt) {
    const float cameraSpeed = 450.f;

    if (IsKeyDown(KEY_W)) {
        camera.target.y -= cameraSpeed * dt;
    } else if (IsKeyDown(KEY_S)) {
        camera.target.y += cameraSpeed * dt;
    }

    if (IsKeyDown(KEY_A)) {
        camera.target.x -= cameraSpeed * dt;
    } else if (IsKeyDown(KEY_D)) {
        camera.target.x += cameraSpeed * dt;
    }
}

void handleZoom(Camera2D& camera, const float dt) {
}