#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif
#define MAX_OBJECTS 50
#define COLLISION_DISTANCE 20

float realRound(float inFloat);

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "raylib");
    SetTargetFPS(60); 

    //Camera Setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Vector3 cameraMovement = { 0.0, 0.0, 0.0 };
    Vector3 cameraRotation = { 0.0, 0.0, 0.0 };
    //Vector3 centerPos = { 0.0, 0.0, 0.0 };

    //Mouse Position Vector in 2D
    Vector2 PrevMousePos = GetMousePosition();
    
    //50x50 plane defined in 4 points
    Vector3 g0 = (Vector3){ -50.0f, 0.0f, -50.0f };
    Vector3 g1 = (Vector3){ -50.0f, 0.0f,  50.0f };
    Vector3 g2 = (Vector3){  50.0f, 0.0f,  50.0f };
    Vector3 g3 = (Vector3){  50.0f, 0.0f, -50.0f };
    Vector3 zeroVec = {0,0,0};

    //Object Lists and Indexers
    Vector3 objList[MAX_OBJECTS] = { 0 };
    int objIndCount = 0;
    Vector3 blueobjList[MAX_OBJECTS] = { 0 };
    int blueobjIndCount = 0;
    Vector3 startCylPos[MAX_OBJECTS] = { 0 };
    int startPosCount = 0;
    Vector3 endCylPos[MAX_OBJECTS] = { 0 };
    int endPosCount = 0;

    //Control bool for drawing
    bool shouldDraw = false;

    //Ball options
    bool red = true;
    bool blue = false;

    //Cylinder Joint
    int inpCounter = 0;
    bool inpButton = false;

    while (!WindowShouldClose())    
    {   
        if (objIndCount == MAX_OBJECTS && blueobjIndCount == MAX_OBJECTS) {
            CloseWindow();
        }

        UpdateCameraPro(&camera, cameraMovement, cameraRotation, 0.0);

        //Custom camera controls
        if (IsKeyPressed(KEY_W)) cameraMovement.x += 1.0f;
        if (IsKeyPressed(KEY_S)) cameraMovement.x -= 1.0f;

        if (IsKeyPressed(KEY_D)) cameraMovement.y += 1.0f;
        if (IsKeyPressed(KEY_A)) cameraMovement.y -= 1.0f;

        if (IsKeyPressed(KEY_Z)) cameraMovement.z += 1.0f;
        if (IsKeyPressed(KEY_X)) cameraMovement.z -= 1.0f;

        if (IsKeyReleased(KEY_W) || IsKeyReleased(KEY_S)) cameraMovement.x = 0.0f;
        if (IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D)) cameraMovement.y = 0.0f;
        if (IsKeyReleased(KEY_Z) || IsKeyReleased(KEY_X)) cameraMovement.z = 0.0f;

        //Convoluted way to get a pseudo-panning effect with the left mouse button
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 MousePos = GetMousePosition();
            Vector2 MouseDelta = Vector2Subtract(MousePos, PrevMousePos);
            PrevMousePos = MousePos;

            if (MouseDelta.x == 0 && MouseDelta.y == 0) {
                cameraRotation.x = 0;
                cameraRotation.y = 0;
            }

            if (MouseDelta.x > 1) {
                cameraRotation.x = -1.5;
            }
            else if (MouseDelta.x < -1) {
                cameraRotation.x = 1.5;
            }
            else if (MouseDelta.x < 1) {
                cameraRotation.x = -MouseDelta.x/10;
            }
            else if (MouseDelta.x > -1) {
                cameraRotation.x = MouseDelta.x/10;
            }
            
            if (MouseDelta.y > 1) {
                cameraRotation.y = -1.5;
            }
            else if (MouseDelta.y < -1) {
                cameraRotation.y = 1.5;
            }
            else if (MouseDelta.y < 1) {
                cameraRotation.y = -MouseDelta.y/10;
            }
            else if (MouseDelta.y > -1) {
                cameraRotation.y = MouseDelta.y/10;
            }
            
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            cameraRotation.x = 0;
            cameraRotation.y = 0;
            cameraRotation.z = 0;
        }
        
        //Buttons that control objects in some way
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            shouldDraw = true;
        }

        if (IsKeyPressed(KEY_E)) {
            red = !red;
            blue = !blue;
        }
        if (IsKeyPressed(KEY_Q)) {
            inpCounter += 1;
            inpButton = true;
            if (inpCounter > 2) {
                inpCounter = 0;
            }
        }
        
        //Ray Casting and Collision
        Ray ray = { 0 };
        ray = GetMouseRay(GetMousePosition(), camera);

        for (int cylInd = 0; cylInd < MAX_OBJECTS; cylInd++) {
            RayCollision redSphereHit = GetRayCollisionSphere(ray, objList[cylInd], 0.75);
            RayCollision blueSphereHit = GetRayCollisionSphere(ray, blueobjList[cylInd], 0.5);
            if (inpButton && inpCounter == 1 && (redSphereHit.hit) && (redSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 startVec = objList[cylInd];
                startCylPos[startPosCount] = startVec;
                startPosCount += 1;
                inpButton = false;
                break;
            }
            else if (inpButton && inpCounter == 2 && (redSphereHit.hit) && (redSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 endVec = objList[cylInd];
                endCylPos[endPosCount] = endVec;
                endPosCount += 1;
                inpCounter = 0;
                inpButton = false;
                break;
            }
            else if (inpButton && inpCounter == 2 && (blueSphereHit.hit) && (blueSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 endVec = blueobjList[cylInd];
                endCylPos[endPosCount] = endVec;
                endPosCount += 1;
                inpCounter = 0;
                inpButton = false;
                break;
            }
            if (inpButton && inpCounter == 1 && (blueSphereHit.hit) && (blueSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 startVec = blueobjList[cylInd];
                startCylPos[startPosCount] = startVec;
                startPosCount += 1;
                inpButton = false;
                break;
            }
            else if (inpButton && inpCounter == 2 && (blueSphereHit.hit) && (blueSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 endVec = blueobjList[cylInd];
                endCylPos[endPosCount] = endVec;
                endPosCount += 1;
                inpCounter = 0;
                inpButton = false;
                break;
            }
            else if (inpButton && inpCounter == 2 && (redSphereHit.hit) && (redSphereHit.distance < COLLISION_DISTANCE)) {
                Vector3 endVec = objList[cylInd];
                endCylPos[endPosCount] = endVec;
                endPosCount += 1;
                inpCounter = 0;
                inpButton = false;
                break;
            }
        }

        //Ray and sphere collisions
        for (int colInd = 0; colInd < MAX_OBJECTS; colInd++) {
            RayCollision redSphereHitInfo = GetRayCollisionSphere(ray, objList[colInd], 0.75);
            RayCollision blueSphereHitInfo = GetRayCollisionSphere(ray, blueobjList[colInd], 0.5);
            if (red) {
                if (shouldDraw && (redSphereHitInfo.hit) && (redSphereHitInfo.distance < COLLISION_DISTANCE)) {
                    Vector3 redVec = {realRound(redSphereHitInfo.point.x), realRound(redSphereHitInfo.point.y), realRound(redSphereHitInfo.point.z)};
                    Vector3 redNorm = {realRound(redSphereHitInfo.normal.x), realRound(redSphereHitInfo.normal.y), realRound(redSphereHitInfo.normal.z)};
                    Vector3 resultantVec = Vector3Add(redVec, redNorm);
                    objList [objIndCount] = resultantVec;
                    objIndCount += 1;
                    shouldDraw = false;
                }
                else if (shouldDraw && (blueSphereHitInfo.hit) && (blueSphereHitInfo.distance < COLLISION_DISTANCE)) {
                    Vector3 blueVec = {realRound(blueSphereHitInfo.point.x), realRound(blueSphereHitInfo.point.y), realRound(blueSphereHitInfo.point.z)};
                    Vector3 blueNorm = {realRound(blueSphereHitInfo.normal.x), realRound(blueSphereHitInfo.normal.y), realRound(blueSphereHitInfo.normal.z)};
                    Vector3 resultantVec = Vector3Add(blueVec, blueNorm);
                    objList [objIndCount] = resultantVec;
                    objIndCount += 1;
                    shouldDraw = false;
                }
            }
            else if (blue) {
                if (shouldDraw && (redSphereHitInfo.hit) && (redSphereHitInfo.distance < COLLISION_DISTANCE)) {
                    Vector3 redVec = {realRound(redSphereHitInfo.point.x), realRound(redSphereHitInfo.point.y), realRound(redSphereHitInfo.point.z)};
                    Vector3 redNorm = {realRound(redSphereHitInfo.normal.x), realRound(redSphereHitInfo.normal.y), realRound(redSphereHitInfo.normal.z)};
                    Vector3 resultantVec = Vector3Add(redVec, redNorm);
                    blueobjList [blueobjIndCount] = resultantVec;
                    blueobjIndCount += 1;
                    shouldDraw = false;
                }
                else if (shouldDraw && (blueSphereHitInfo.hit) && (blueSphereHitInfo.distance < COLLISION_DISTANCE)) {
                    Vector3 blueVec = {realRound(blueSphereHitInfo.point.x), realRound(blueSphereHitInfo.point.y), realRound(blueSphereHitInfo.point.z)};
                    Vector3 blueNorm = {realRound(blueSphereHitInfo.normal.x), realRound(blueSphereHitInfo.normal.y), realRound(blueSphereHitInfo.normal.z)};
                    Vector3 resultantVec = Vector3Add(blueVec, blueNorm);
                    blueobjList [blueobjIndCount] = resultantVec;
                    blueobjIndCount += 1;
                    shouldDraw = false;
                }
            }
        }

        //Handling all Collisions on a 50x50 grid centered at (0,0)
        RayCollision groundHitInfo = GetRayCollisionQuad(ray, g0, g1, g2, g3);
        if (shouldDraw && (groundHitInfo.hit) && (groundHitInfo.distance < COLLISION_DISTANCE)) {
            Vector3 rayVec = {realRound(groundHitInfo.point.x), realRound(groundHitInfo.point.y), realRound(groundHitInfo.point.z)};
            if (red) {
                for (int k = 0; k < MAX_OBJECTS; k++) {
                    if (Vector3Equals(rayVec, objList[k]) || Vector3Equals(rayVec, blueobjList[k])) {
                        shouldDraw = false;
                        break;
                    }
                }
                if (shouldDraw && objIndCount < MAX_OBJECTS) {
                    objList [objIndCount] = rayVec;
                    objIndCount += 1;
                    shouldDraw = false;
                }
                else {
                    shouldDraw = false;
                }
            }
            else if (blue) {
                for (int k = 0; k < MAX_OBJECTS; k++) {
                    if (Vector3Equals(rayVec, objList[k]) || Vector3Equals(rayVec, blueobjList[k])) {
                        shouldDraw = false;
                        break;
                    } 
                }
                if (shouldDraw && MAX_OBJECTS) {
                    blueobjList [blueobjIndCount] = rayVec;
                    blueobjIndCount += 1;
                    shouldDraw = false;
                }
                else {
                    shouldDraw = false;
                }
            }
        }
        else {
            shouldDraw = false;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                //Drawing all shapes
                for (int i = 0; i < MAX_OBJECTS; i++) {
                    if ((objList[i].x == 0 && objList[i].y == 0 && objList[i].z == 0)) {
                        continue;
                    }
                    DrawSphere(objList[i], 0.75f, RED);
                }

                for (int i = 0; i < MAX_OBJECTS; i++) {
                    if ((blueobjList[i].x == 0 && blueobjList[i].y == 0 && blueobjList[i].z == 0)) {
                        continue;
                    }
                    DrawSphere(blueobjList[i], 0.5f, BLUE);
                }

                for (int i = 0; i < MAX_OBJECTS; i++) {
                    if (Vector3Equals(startCylPos[i], zeroVec) || Vector3Equals(endCylPos[i], zeroVec)) {
                        continue;
                    }
                    DrawCylinderWiresEx(startCylPos[i], endCylPos[i], 0.25, 0.25, 50, BLACK);
                }

                DrawGrid(80, 1.0f);

            EndMode3D();
            //DrawText(TextFormat("- Position: (%d)", inpCounter), 610, 60, 10, BLACK);

        EndDrawing();

    }
    CloseWindow();

    return 0;
}

float realRound(float inFloat) {
    float roundedF = (float) round(inFloat);
    float unroundedF = inFloat;

    float difference = unroundedF - roundedF;
    float returnF = 0.0;

    if (difference < 0.5) {
        returnF = roundedF;
    }
    else if (difference >= 0.5) {
        float recipDiff = 1 - difference;
        returnF = unroundedF + recipDiff;
    }
    else {
        printf("huh?");
    }
    return returnF;
}
