/**********************************************************************************************
*
*   raylibExtras * Utilities and Shared Components for Raylib
*
*   RLAssets * Simple Asset Managment System for Raylib
*
*   LICENSE: MIT
*
*   Copyright (c) 2020 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

#include "raylib.h"
#include "raymath.h"



Camera ViewCam = { 0 };
Vector2 LastMousePos = { 0 };

Vector3 previousCameraPosition = { 0.0f, 0.0f, 0.0f };

float MoveSpeed = 10;

int ForwardKey = KEY_W;
int BackwardKey = KEY_S;
int LeftKey = KEY_A;
int RightKey = KEY_D;

enum CameraMode
{
    EditMode,
    FPSMode,
    LockedMode,
};

void InitLizardFreeCam(float fovy)
{
    ViewCam.fovy = fovy;
    ViewCam.projection = CAMERA_PERSPECTIVE;
    ViewCam.up.y = 1;
    ViewCam.target.z = 1;
    
    LastMousePos = GetMousePosition(); 
}

void UpdateLizardFreeCam(int MODE, Vector3 POS)
{
    previousCameraPosition = ViewCam.position;
    static float pitch = 0.0f;
    static float yaw = 0.0f;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || MODE == FPSMode)
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            MoveSpeed = 33;
            if (IsKeyDown(KEY_X))
            {
                MoveSpeed = 9999;
            }
        }
        else
        {
            MoveSpeed = 10;
        }

        float degPerPixel = ViewCam.fovy / GetScreenHeight();
        Vector2 delta = Vector2Scale(Vector2Subtract(LastMousePos, GetMousePosition()), degPerPixel);

        // Update pitch and yaw
        pitch += delta.y;  // Change the sign to correct the inversion
        yaw += delta.x;

        // Clamp the pitch to prevent looking all the way up or down
        float pitchLimit = 89.0f;
        if (pitch > pitchLimit) pitch = pitchLimit;
        if (pitch < -pitchLimit) pitch = -pitchLimit;

        // Calculate forward direction based on pitch and yaw
        Vector3 forward;
        forward.x = cosf(DEG2RAD * pitch) * sinf(DEG2RAD * yaw);
        forward.y = sinf(DEG2RAD * pitch);
        forward.z = cosf(DEG2RAD * pitch) * cosf(DEG2RAD * yaw);
        forward = Vector3Normalize(forward);

        Vector3 right = Vector3CrossProduct(forward, (Vector3) { 0, 1, 0 });
        right = Vector3Normalize(right);
        Vector3 up = Vector3CrossProduct(right, forward);
        up = Vector3Normalize(up);

        float forwardSpeed = 0;
        if (IsKeyDown(ForwardKey))
            forwardSpeed = 1;
        else if (IsKeyDown(BackwardKey))
            forwardSpeed = -1;

        float sideSpeed = 0;
        if (IsKeyDown(RightKey))
            sideSpeed = 1;
        else if (IsKeyDown(LeftKey))
            sideSpeed = -1;

        float upSpeed = 0;
        if (IsKeyDown(KEY_SPACE))
            upSpeed = 1;
        else if (IsKeyDown(KEY_LEFT_CONTROL))
            upSpeed = -1;

        forwardSpeed *= GetFrameTime() * MoveSpeed;
        sideSpeed *= GetFrameTime() * MoveSpeed;
        upSpeed *= GetFrameTime() * MoveSpeed;

        if (MODE == EditMode)
        {
            ViewCam.position = Vector3Add(ViewCam.position, Vector3Scale(forward, forwardSpeed));
            ViewCam.position = Vector3Add(ViewCam.position, Vector3Scale(right, sideSpeed));
            ViewCam.position.y += upSpeed; //Vector3Add(ViewCam.position, Vector3Scale(up, upSpeed));
        }
        else if (MODE == FPSMode)
        {
            ViewCam.position = POS;
            DisableCursor();
        }

        ViewCam.target = Vector3Add(ViewCam.position, forward);
    }
    LastMousePos = GetMousePosition();
    
}

