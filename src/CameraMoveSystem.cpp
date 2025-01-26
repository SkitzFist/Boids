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
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel == 0) {
        return;
    }

    // Get the current world position of the mouse before zooming
    Vector2 prevMouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    float zoomStrength = camera.zoom > 0.03f ? 0.01f : 0.005f;
    float zoom = mouseWheel < 0 ? -zoomStrength : zoomStrength;
    camera.zoom += zoom;

    moveTowardsMouse(camera);

    Vector2 newMouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    camera.target.x += (prevMouseWorldPos.x - newMouseWorldPos.x);
    camera.target.y += (prevMouseWorldPos.y - newMouseWorldPos.y);
}

void moveTowardsMouse(Camera2D& camera) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    // Calculate the screen's center in world coordinates
    Vector2 screenCenterWorld = GetScreenToWorld2D((Vector2){GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f}, camera);

    // Adjust the camera's target
    camera.target.x = mouseWorldPos.x - (screenCenterWorld.x - camera.target.x);
    camera.target.y = mouseWorldPos.y - (screenCenterWorld.y - camera.target.y);
}