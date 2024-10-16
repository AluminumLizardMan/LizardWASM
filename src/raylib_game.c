/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "LizardFreeCamera.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 


static const int screenWidth = 1280;
static const int screenHeight = 720;
static RenderTexture2D target = { 0 };


void UpdateGame(void)
{
    UpdateLizardFreeCam(EditMode, Vector3Zero());

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        LastMousePos = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
        HideCursor();
        SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
        //DisableCursor();
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
    {
        SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
        //EnableCursor();
        ShowCursor();
    }

    BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(ViewCam);

        DrawCube(Vector3Zero(), 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires(Vector3Zero(), 2.0f, 2.0f, 2.0f, MAROON);

        DrawGrid(10, 1.0f);

        EndMode3D();

        DrawRectangle(10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(10, 10, 320, 93, BLUE);

        DrawText("Free camera default controls:", 20, 20, 10, BLACK);
        DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
        DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
        DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

    EndTextureMode();
    
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        if (IsKeyDown(KEY_F))
        {
            DrawText("Hopefully it works - Lizard, 2024", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
        }
        else
        {
            DrawText("Hopefully it works - Lizard, 2024", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, BLUE);
        }

    EndDrawing();
}



int main(void)
{
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    InitLizardFreeCam(70.0f);


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateGame, 60, 1);
#else
    SetTargetFPS(60);     
    while (!WindowShouldClose())
    {
        UpdateGame();
    }
#endif

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}