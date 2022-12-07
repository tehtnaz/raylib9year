/*******************************************************************************************
*
*   raylib 9years gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 4.5-dev
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    //#define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include "emscripten.h"      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 


#include "dataHandling/dataHandling.h"

#define PHYSAC_IMPLEMENTATION
#include "physac.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

//----------------------------------------------------------------------------------
// Constant values (#define)
//----------------------------------------------------------------------------------

#define INPUT_VELOCITY 0.1f

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 256;
static const int screenHeight = 256;

static unsigned int screenScale = 1; 
static unsigned int prevScreenScale = 1;

static RenderTexture2D target = { 0 };  // Initialized at init

static PhysicsBody player;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib 9yr gamejam");
    InitPhysics();

    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    // Init physics
    SetPhysicsGravity(0, 0);
    
    // Init player, disable dynamics
    player = CreatePhysicsBodyCircle((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, 8, 1);
    player->enabled = true;
    player->freezeOrient = true;

    PhysicsAddTorque(CreatePhysicsBodyRectangle((Vector2){75, 20}, 20, 20, 1), 100);

    parseStructGroupInfo(readFileSF("./../res/first.sf"));



#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    CloseWindow();        // Close window and OpenGL context


    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------

// Get vector2 force from key input
Vector2 GetKeyInputForce(void){
    Vector2 vec2 = {0, 0};

    if(IsKeyDown(KEY_D)){
        vec2.x += INPUT_VELOCITY;
    }
    if(IsKeyDown(KEY_A)){
        vec2.x += -INPUT_VELOCITY;
    }
    if(IsKeyDown(KEY_S)){
        vec2.y += INPUT_VELOCITY;
    }
    if(IsKeyDown(KEY_W)){
        vec2.y += -INPUT_VELOCITY;
    }

    return vec2;
}

float Vector2Mag(Vector2 v2){
    return sqrtf(v2.x * v2.x + v2.y * v2.y);
}

void DrawPhysicsBody(int index, Color color){
    PhysicsBody body = GetPhysicsBody(index);
    if(body->shape.type == PHYSICS_CIRCLE){
        
        DrawCircleV(body->position, body->shape.radius, color);
    }else{
        // You're a polygon I guess

        float rotation = acos(body->shape.transform.m00);
        Vector2 vertex = MathVector2Subtract(GetPhysicsShapeVertex(body, 0), body->position);
        
        DrawPoly(body->position, body->shape.vertexData.vertexCount, Vector2Mag(vertex), rotation * RAD2DEG + 45, color);

        int vertexCount = GetPhysicsShapeVerticesCount(index);
        for (int j = 0; j < vertexCount; j++)
        {
            // Get physics bodies shape vertices to draw lines
            // Note: GetPhysicsShapeVertex() already calculates rotation transformations
            Vector2 vertexA = GetPhysicsShapeVertex(body, j);

            int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
            Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

            DrawLineV(vertexA, vertexB, GREEN);     // Draw a line between two vertex positions
        }

        
        
    }
}


// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // Screen scale logic (x2)
    if (IsKeyPressed(KEY_ONE)) screenScale = 1;
    else if (IsKeyPressed(KEY_TWO)) screenScale = 2;
    else if (IsKeyPressed(KEY_THREE)) screenScale = 3;
    
    if (screenScale != prevScreenScale)
    {
        // Scale window to fit the scaled render texture
        SetWindowSize(screenWidth*screenScale, screenHeight*screenScale);
        
        // Scale mouse proportionally to keep input logic inside the 256x256 screen limits
        SetMouseScale(1.0f/(float)screenScale, 1.0f/(float)screenScale);
        
        prevScreenScale = screenScale;
    }

    // 
    //----------------------------------------------------------------------------------
    player->velocity = GetKeyInputForce();
    int bodyCount = GetPhysicsBodiesCount();
    for (int i = bodyCount - 1; i >= 0; i--)
    {
        PhysicsBody body = GetPhysicsBody(i);
        if (body != NULL && (body->position.y > screenHeight*2)) DestroyPhysicsBody(body);
    }

    //Physics
    UpdatePhysics();
    //LOG("%f %f\n", player->velocity.x, player->velocity.y);

    // Draw
    //----------------------------------------------------------------------------------
    // Render all screen to texture (for scaling)
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        
        // TODO: Draw screen at 256x256
        DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, SKYBLUE);
        
        bodyCount = GetPhysicsBodiesCount();
        for(int i = 0; i < bodyCount; i++){
            DrawPhysicsBody(i, RED);
        }

        #if defined(_DEBUG)
            // Draw equivalent mouse position on the target render-texture
            DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);
        #endif

    EndTextureMode();
    
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen scaled as required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width*screenScale, (float)target.texture.height*screenScale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        

    EndDrawing();
    //----------------------------------------------------------------------------------  
}