#ifndef BOIDS_DEBUG_MORTON_VIEW
#define BOIDS_DEBUG_MORTON_VIEW

#include "raylib.h"

#include "MortonCodeMap.h"

void drawMortonCodes(const MortonMap& mortonMap) {

    const float offset = 10.f;
    Rectangle frameRect = {
        GetScreenWidth() * 0.75f - offset, offset,
        GetScreenWidth() / 4.f - offset * 2.f, GetScreenHeight() - offset * 2.f};

    Color frameBgColor = RAYWHITE;
    frameBgColor.a = (unsigned char)150;

    DrawRectangleRounded(frameRect, 0.1f, 16, frameBgColor);

    std::string ids = "";
    std::string mortonCodes = "";
    std::string idsSorted = "";
    std::string mortonCodesSorted = "";

    for (std::size_t i = 0; i < mortonMap.entries.size(); ++i) {
        ids += std::to_string(i) + "\n";
        mortonCodes += std::to_string(mortonMap.entries[i]) + "\n";

        idsSorted += std::to_string(mortonMap.ids[i]) + "\n";
        mortonCodesSorted += std::to_string(*mortonMap.entriesPtr[i]) + "\n";
    }

    const float fontSize = 32.f;
    const float spacing = 1.5f;
    const float textOffset = 25.f;
    const float separatorOffset = 15.f;

    const Vector2 idPos = {frameRect.x + textOffset, frameRect.y + textOffset};
    Vector2 textSize = MeasureTextEx(GetFontDefault(), ids.c_str(), fontSize, spacing);
    float separatorX = idPos.x + textSize.x + separatorOffset;
    DrawLine(separatorX, frameRect.y, separatorX, frameRect.height, DARKGRAY);

    const Vector2 mortonCodesPos = {separatorX + textOffset / 2.f, idPos.y};
    textSize = MeasureTextEx(GetFontDefault(), mortonCodes.c_str(), fontSize, spacing);
    separatorX = frameRect.x + (frameRect.width / 2);
    DrawLineEx({separatorX, frameRect.y}, {separatorX, frameRect.y + frameRect.height}, 3.f, BLACK);

    const Vector2 idsSortedPos = {separatorX + textOffset, idPos.y};
    textSize = MeasureTextEx(GetFontDefault(), idsSorted.c_str(), fontSize, spacing);
    separatorX = idsSortedPos.x + textSize.x + separatorOffset;
    DrawLine(separatorX, frameRect.y, separatorX, frameRect.y + frameRect.height, BLACK);

    const Vector2 mortonCodesSortedPos = {separatorX + textOffset / 2.f, idPos.y};
    textSize = MeasureTextEx(GetFontDefault(), mortonCodesSorted.c_str(), fontSize, spacing);

    DrawTextEx(GetFontDefault(), ids.c_str(), idPos, fontSize, spacing, BLACK);
    DrawTextEx(GetFontDefault(), mortonCodes.c_str(), mortonCodesPos, fontSize, spacing, BLACK);
    DrawTextEx(GetFontDefault(), idsSorted.c_str(), idsSortedPos, fontSize, spacing, BLACK);
    DrawTextEx(GetFontDefault(), mortonCodesSorted.c_str(), mortonCodesSortedPos, fontSize, spacing, BLACK);
}

#endif
