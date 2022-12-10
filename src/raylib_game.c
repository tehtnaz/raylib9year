#include "raylib.h"

#if defined(PLATFORM_WEB)
    //#define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include "emscripten.h"      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 

#include "displayText.h"
#include "triggers.h"
#include "animation.h"


#include "dataHandling/dataHandling.h"

#define PHYSAC_DEBUG
#define PHYSAC_IMPLEMENTATION
#include "physac.h"

//----------------------------------------------------------------------------------
// Defines and Macros (Constant values)
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#define INPUT_VELOCITY 0.1f
#define MAX_BUTTONS 8

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 256;
static const int screenHeight = 256;

static unsigned int screenScale = 1; 
static unsigned int prevScreenScale = 1;

static RenderTexture2D target = { 0 };  // Initialized at init

static Vector2 startingPos = {128, 128};
static Texture2D playerTexture;
static PhysicsBody player;
static PhysicsBody playerGrabZone;

static Animation haroldTextBox;
static Animation menuAnimation;

static int buttonCount;
static Animation buttonArray[MAX_BUTTONS];
static Vector2 buttonPosition[MAX_BUTTONS];


static int levelSelect = 0;
static Texture2D levelBackground;

static GameScreen currentScreen = SCREEN_TITLE;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

static void UpdateDrawFrame(void);      // Update and Draw one frame
Vector2 GetKeyInputForce(Vector2 preInput);
float Vector2Mag(Vector2 v2);
void DrawPhysicsBody(int index, Color color);

//--------------------------------------------------------------------------------------------
// Trigger functions Declaration
//--------------------------------------------------------------------------------------------

void AddPlayerInputForce(PhysicsBody body);
void DrawHaroldText(const char** texts, int textCount);
void LoadNextLevel();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void){
    #if !defined(_DEBUG)
        SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
    #endif
    printf("%s\n", GetApplicationDirectory());

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Grayzone");
    InitPhysics();

    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    // Init physics
    SetPhysicsGravity(0, 0);
    SetPhysicsAirFriction(0.01f, 0.01f);

    // Init player, disable dynamics
    int tags[8] = {0};
    tags[0] = 2;
    tags[1] = 3;
    player = CreatePhysicsBodyCircle((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, 8, 1, tags, 2, 0);
    player->enabled = true;
    player->freezeOrient = true;

    playerGrabZone = CreatePhysicsBodyCircle(player->position, 22, 1, 0, 0, 1);
    playerGrabZone->enabled = false;
    playerGrabZone->freezeOrient = true;

    //CreatePhysicsBodyCircle((Vector2){ screenWidth, screenHeight/2.0f }, 8, 1, 0, 2)->enabled = false;

    

    playerTexture = LoadTexture("./../res/Characters/player/front.png");

    haroldTextBox = assignProperties(0, 0, 2, true, 2, true);
    haroldTextBox = getFromFolder(haroldTextBox, "./../res/text/harold/", true);

    menuAnimation = assignProperties(0, 0, 1, true, 5, true);
    menuAnimation = getFromFolder(menuAnimation, "./../res/Levels/progression/", true);
    

    //QueueDisplayText("My name is Walter Hartwell White. I live at 308 Negra Arroyo Lane, Albuquerque, New Mexico, 87104. This is my confession. If you're watching this tape, I'm probably dead, murdered by my brother-in-law Hank Schrader. Hank has been building a Virtual Youtuber empire for over a year now and using me as his recruiter. Shortly after my 50th birthday, Hank came to me with a rather, shocking proposition. He asked that I use my Live2D knowledge to recruit talents, which he would then hire using his connections in the Japanese utaite world. Connections that he made through his career with Niconico. I was... astounded, I... I always thought that Hank was a very moral man", (Vector2){5,5},246);

    LoadNextLevel();

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
    #else
        SetTargetFPS(60);     // Set our game frames-per-second
        //--------------------------------------------------------------------------------------

        // Main game loop
        while (!WindowShouldClose()){
            UpdateDrawFrame();
        }
    #endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    ClearDisplayText();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------

// Update and draw frame
static void UpdateDrawFrame(void)
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


    
    
    if(currentScreen == SCREEN_TITLE){
        if(IsKeyPressed(KEY_SPACE)){
            currentScreen = SCREEN_GAMEPLAY;
        }
    }
    if(currentScreen == SCREEN_GAMEPLAY){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)){
            ClearDisplayText();
        }
        // 
        //----------------------------------------------------------------------------------
        player->velocity = GetKeyInputForce(player->velocity);
        playerGrabZone->position = player->position;
        int bodyCount = GetPhysicsBodiesCount();
        for (int i = bodyCount - 1; i >= 0; i--)
        {
            PhysicsBody body = GetPhysicsBody(i);
            if (body != NULL && (body->position.y > screenHeight*2 || body->position.y < -256)) body->position = (Vector2){128, 128};
            // DestroyPhysicsBody(body);
        }
        //Physics
        ActivateAllContactedTriggers();
        UpdatePhysics();
    }

    // Draw
    //----------------------------------------------------------------------------------
    // Render all screen to texture (for scaling)
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        if(currentScreen == SCREEN_TITLE){
            DrawAnimationPro(&menuAnimation,(Vector2){0,0}, 1, WHITE, CYCLE_FORWARD);
            DrawRectangle(0, 0, 256, 256, (Color){15, 15, 15, 240});
            DrawText("Grayzone", 10, 10, 40, GRAY);
            DrawText("Press space to start", 10, 55, 10, WHITE);
        }
        if(currentScreen == SCREEN_GAMEPLAY){
            DrawTexture(levelBackground, 0, 0, WHITE);
            
            for(int i = 0; i < buttonCount; i++){
                DrawAnimationPro(&(buttonArray[i]), buttonPosition[i], 1, WHITE, CYCLE_NONE);
            }

            int bodyCount = GetPhysicsBodiesCount();
            for(int i = 0; i < bodyCount; i++){
                DrawPhysicsBody(i, (Color){ 230, 41, 55, 127 });
            }
            DrawTextureEx(playerTexture, (Vector2){player->position.x - 7, player->position.y - 15}, 0, 1, WHITE);

            if(GetDisplayTextEnabled()) DrawAnimationPro(&haroldTextBox, (Vector2){21, 2}, 1, WHITE, CYCLE_FORWARD);
            UpdateAndDrawTypingText(WHITE);

            #if defined(_DEBUG)
                // Draw equivalent mouse position on the target render-texture
                DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);
            #endif
        }
    EndTextureMode();
    
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen scaled as required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width*screenScale, (float)target.texture.height*screenScale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        

    EndDrawing();
    //----------------------------------------------------------------------------------  
}

// Get vector2 force from key input
Vector2 GetKeyInputForce(Vector2 preInput){
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

    if(vec2.x == 0){
        vec2.x = preInput.x;
    }
    if(vec2.y == 0){
        vec2.y = preInput.y;
    }

    return vec2;
}

float Vector2Mag(Vector2 v2){
    return sqrtf(v2.x * v2.x + v2.y * v2.y);
}


void DrawPhysicsBody(int index, Color color){
    #ifdef _DEBUG
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
    #endif
}


//--------------------------------------------------------------------------------------------
// Trigger functions definition
//--------------------------------------------------------------------------------------------

void AddPlayerInputForce(PhysicsBody body){
    if(IsKeyDown(KEY_E))body->velocity = player->velocity;
}

void DrawHaroldText(const char** texts, int textCount){
    printf("Queued: %d\n", textCount);
    for(int i = 0; i < textCount; i++){
        QueueDisplayText(texts[i], (Vector2){79, 9}, 150);
    }
}

void LoadNextLevel(){
    LOG("DEBUG: Attemping to load next level...\n");
    if(levelSelect != 0) UnloadTexture(levelBackground);
    levelBackground = LoadTexture(TextFormat("./../res/Levels/grayzone_level%d.png", levelSelect + 1));

    int bodyCount = GetPhysicsBodiesCount();
    for(int i = 0; i < bodyCount - 2; i++){
        DestroyPhysicsBody(GetPhysicsBody(2));
    }
    ClearDisplayTextQueue();
    ClearDisplayText();

    ResetAllTriggers();

    // Pre defined TriggerEvents
    NewTriggerEvent(1, false, CreateTriggerEventFunctionData_SetForce(AddPlayerInputForce));
    NewTriggerEvent(3, true, CreateTriggerEventFunctionData_Function(LoadNextLevel));

    parseStructGroupInfo(readFileSF(TextFormat("./../res/level-files/%d.sf", levelSelect + 1)), DrawHaroldText, &startingPos);

    player->position = startingPos;

    levelSelect++;
}