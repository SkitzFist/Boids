#ifndef BOIDS_DEBUG_TILE_INDEX_H
#define BOIDS_DEBUG_TILE_INDEX_H

#include <string>

#include "raylib.h"

#include "TileMapSimd.h"

void draw(const TileMap& tileMap) {

    constexpr const float offset = 10.f;
    Rectangle frameRect = {
        GetScreenWidth() * 0.9f - offset, offset,
        GetScreenWidth() * 0.1f - offset * 2.f, GetScreenHeight() - offset * 2.f};

    Color frameBgColor = RAYWHITE;
    frameBgColor.a = (unsigned char)150;

    std::string ids = "";
    std::string indexes = "";

    for (int i = 0; i < tileMap.tiles.size(); ++i) {
        ids += std::to_string(i) + "\n";
        indexes += std::to_string(tileMap.tiles[i]) + "\n";
    }

    constexpr const float fontSize = 32.f;
    constexpr const float spacing = 1.5f;
    constexpr const float textOffset = 25.f;
    constexpr const float separatorOffset = 15.f;

    const Vector2 idPos = {frameRect.x + textOffset, frameRect.y + textOffset};
    Vector2 textSize = MeasureTextEx(GetFontDefault(), ids.c_str(), fontSize, spacing);
    float separatorX = idPos.x + textSize.x + separatorOffset;

    const Vector2 indexesPos = {separatorX + textOffset / 2.f, idPos.y};
    textSize = MeasureTextEx(GetFontDefault(), indexes.c_str(), fontSize, spacing);

    DrawRectangleRounded(frameRect, 0.1f, 16, frameBgColor);
    DrawTextEx(GetFontDefault(), ids.c_str(), idPos, fontSize, spacing, BLACK);
    DrawTextEx(GetFontDefault(), indexes.c_str(), indexesPos, fontSize, spacing, BLACK);

    DrawLine(separatorX, frameRect.y, separatorX, frameRect.height, DARKGRAY);
}

#endif
