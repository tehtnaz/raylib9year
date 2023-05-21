#include "raylib.h"

#if defined(PLATFORM_WEB)
    //#define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include "emscripten.h"      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdlib.h>                         // Required for: 

#include "displayText.h"
#include "triggers.h"
#include "animation.h"
#include "levelObjects.h"

// TODO:
    // Portal disables (disable button while not in dimension)
    // Door Opening functionality
    // Death animation (fade to black)
    // Dimension colour flashes during last 3 seconds
    // Blue dimension prevents death


#include "dataHandling/dataHandling.h"

#define PHYSAC_DEBUG
#define PHYSAC_IMPLEMENTATION
#include "physac.h"

//----------------------------------------------------------------------------------
// Defines and Macros (Constant values)
//----------------------------------------------------------------------------------

#define INPUT_VELOCITY 0.1f

#define DIMENSION_RED       (Color){244, 26, 26, 73}
#define DIMENSION_BLUE      (Color){40, 92, 196, 104}
#define DIMENSION_GREEN     (Color){36, 82, 59, 70}
#define DIMENSION_YELLOW    (Color){255, 252, 64, 49}


//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen{
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
static PhysicsBody player;
static PhysicsBody playerGrabZone;

static Texture2D playerBackward;
static Texture2D playerForward;
static Animation playerRunBackward;
static Animation playerRunForward;

static Animation haroldTextBox;
static Animation menuAnimation;

static int levelSelect = 0;
static Texture2D levelBackground;

static GameScreen currentScreen = SCREEN_TITLE;

static Music titleMusic;
static Music gameMusic;

static int currentDimension; //dimension / 0 = normal; 1 = red; 2 = blue; 3 = green; 4 = yellow
static float dimensionTimer = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

static void UpdateDrawFrame(void);      // Update and Draw one frame
Vector2 GetKeyInputForce(Vector2 preInput, int currentDimension);
float Vector2Mag(Vector2 v2);
void DrawPhysicsBody(int index, Color color);
Color GetDimensionColour(int dimension);

//--------------------------------------------------------------------------------------------
// Trigger functions Declaration
//--------------------------------------------------------------------------------------------

void AddPlayerInputForce(PhysicsBody body);
void DrawHaroldText(const char** texts, int textCount);
void LoadNextLevel();
void StartDeathAnimation();
void HitButton_DestroyPortalTrigger();
void HitPortal_DestroyButtonTrigger();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void){
    #if !defined(_DEBUG)
        SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
    #else
        SetTraceLogLevel(LOG_DEBUG);
    #endif
    TraceLog(LOG_DEBUG, "Loading from: %s", GetApplicationDirectory());

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Grayzone");
    InitAudioDevice();
    InitPhysics();
    SetMasterVolume(0.2f);

    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    // Init physics
    SetPhysicsGravity(0, 0);
    SetPhysicsAirFriction(0.01f, 0.01f);

    // Init player, disable dynamics
    player = CreatePhysicsBodyCircle((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, 8, 1, 0);
    AddTagToPhysicsBody(player, 2);
    AddTagToPhysicsBody(player, 3);
    player->enabled = true;
    player->freezeOrient = true;

    playerGrabZone = CreatePhysicsBodyCircle(player->position, 22, 1, 1);
    playerGrabZone->enabled = false;
    playerGrabZone->freezeOrient = true;

    //CreatePhysicsBodyCircle((Vector2){ screenWidth, screenHeight/2.0f }, 8, 1, 0, 2)->enabled = false;


    LevelObjectsInit();

    playerBackward = LoadTexture("./../res/Characters/player/backward.png");
    playerForward = LoadTexture("./../res/Characters/player/forward.png");

    playerRunBackward = assignProperties(0, 0, 8, true, 3, false);
    playerRunBackward = GetAnimationFromFolder(playerRunBackward, true, "./../res/Characters/player/running_backward/");
    playerRunForward = assignProperties(0, 0, 8, true, 3, false);
    playerRunForward = GetAnimationFromFolder(playerRunForward, true, "./../res/Characters/player/running_forward/");

    haroldTextBox = assignProperties(0, 0, 2, true, 2, true);
    haroldTextBox = GetAnimationFromFolder(haroldTextBox, true, "./../res/Text/harold/");

    menuAnimation = assignProperties(0, 0, 1, true, 5, true);
    menuAnimation = GetAnimationFromFolder(menuAnimation, true, "./../res/Levels/progression/");

    titleMusic = LoadMusicStream("./../res/Music/bosstheme.mp3");
    gameMusic = LoadMusicStream("./../res/Music/normallevel.mp3");

    //QueueDisplayText("My name is Walter Hartwell White. I live at 308 Negra Arroyo Lane, Albuquerque, New Mexico, 87104. This is my confession. If you're watching this tape, I'm probably dead, murdered by my brother-in-law Hank Schrader. Hank has been building a Virtual Youtuber empire for over a year now and using me as his recruiter. Shortly after my 50th birthday, Hank came to me with a rather, shocking proposition. He asked that I use my Live2D knowledge to recruit talents, which he would then hire using his connections in the Japanese utaite world. Connections that he made through his career with Niconico. I was... astounded, I... I always thought that Hank was a very moral man", (Vector2){5,5},246);

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
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
    
    #if defined(_DEBUG)
    if(IsKeyPressed(KEY_I)){
        currentDimension++;
        if(currentDimension > 4) currentDimension = 0;
    }
    #endif
    if(currentDimension > 0){
        dimensionTimer -= GetFrameTime();
        if(dimensionTimer < 0) currentDimension = 0;
    }
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
            LoadNextLevel();
        }else{
            if(!IsMusicStreamPlaying(titleMusic)){
                PlayMusicStream(titleMusic);
            }
            UpdateMusicStream(titleMusic);
        }
    }
    if(currentScreen == SCREEN_GAMEPLAY){
        if(IsMusicStreamPlaying(titleMusic)){
            StopMusicStream(titleMusic);
        }
        if(!IsMusicStreamPlaying(gameMusic)){
            PlayMusicStream(gameMusic);
        }
        UpdateMusicStream(gameMusic);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)){
            ClearDisplayText();
        }
        // 
        //----------------------------------------------------------------------------------
        player->velocity = GetKeyInputForce(player->velocity, currentDimension);
        playerGrabZone->position = player->position;
        int bodyCount = GetPhysicsBodiesCount();
        for (int i = bodyCount - 1; i >= 0; i--)
        {
            PhysicsBody body = GetPhysicsBody(i);
            if (body != NULL && (body->position.y > screenHeight*2 || body->position.y < -256)) body->position = (Vector2){128, 128};
            // DestroyPhysicsBody(body);
        }
        UpdateDoors();
        //Physics
        UpdatePhysics();
        UpdateAndActivateTriggers();
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
            
            RenderLevelObjects();

            int bodyCount = GetPhysicsBodiesCount();
            for(int i = 0; i < bodyCount; i++){
                DrawPhysicsBody(i, (Color){ 230, 41, 55, 127 });
            }
            Vector2 input = GetKeyInputForce((Vector2){0, 0}, 0);
            if(input.x != 0 || input.y != 0){
                DrawAnimationPro(player->velocity.y < 0 ? &playerRunForward : &playerRunBackward, (Vector2){player->position.x - 7, player->position.y - 25}, 1, WHITE, CYCLE_SHAKE);
            }else{
                DrawTextureEx(player->velocity.y < 0 ? playerForward : playerBackward, (Vector2){player->position.x - 7, player->position.y - 25}, 0, 1, WHITE);
            }

            if(currentDimension != 0) DrawRectangle(0, 0, 256, 256, GetDimensionColour(currentDimension));

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

        #if defined(_DEBUG)
            DrawText(TextFormat("x:%d\ny:%d", GetMouseX(), GetMouseY()), 0, 0, 40, WHITE);
        #endif
    EndDrawing();
    //----------------------------------------------------------------------------------  
}

// Get vector2 force from key input
Vector2 GetKeyInputForce(Vector2 preInput, int currentDimension){
    Vector2 vec2 = {0, 0};
    float multiplier = 1;

    if(currentDimension == 1){
        multiplier = 2.0f;
    }else if(currentDimension == 2){
        multiplier = 0.5f;
    }
    if(IsKeyDown(KEY_D)){
        vec2.x += INPUT_VELOCITY * multiplier;
    }
    if(IsKeyDown(KEY_A)){
        vec2.x += -INPUT_VELOCITY * multiplier;
    }
    if(IsKeyDown(KEY_S)){
        vec2.y += INPUT_VELOCITY * multiplier;
    }
    if(IsKeyDown(KEY_W)){
        vec2.y += -INPUT_VELOCITY * multiplier;
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

        //float rotation = acos(body->shape.transform.m00);
        //Vector2 vertex = MathVector2Subtract(GetPhysicsShapeVertex(body, 0), body->position);
        
        //DrawPoly(body->position, body->shape.vertexData.vertexCount, Vector2Mag(vertex), rotation * RAD2DEG + 45, color);

        
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
            //DrawText(TextFormat("%d, %d", body->position.x, body->position.y), body->position.x, body->position.y, 20, WHITE);
        
    }
    #endif
}

Color GetDimensionColour(int dimension){
    switch(dimension){
        case 1: return DIMENSION_RED;
        case 2: return DIMENSION_BLUE;
        case 3: return DIMENSION_GREEN;
        case 4: return DIMENSION_YELLOW;
        default: return WHITE;
    }
}


//--------------------------------------------------------------------------------------------
// Trigger functions definition
//--------------------------------------------------------------------------------------------

void AddPlayerInputForce(PhysicsBody body){
    if(IsKeyDown(KEY_E))body->velocity = player->velocity;
}

void DrawHaroldText(const char** texts, int textCount){
    TraceLog(LOG_DEBUG, "Queued: %d", textCount);
    for(int i = 0; i < textCount; i++){
        QueueDisplayText(texts[i], (Vector2){79, 9}, 150);
    }
}

void StartDeathAnimation(){
    TraceLog(LOG_DEBUG, "StartDeathAnimation() - Killed player");
    player->position = startingPos;
}

void HitButton_DestroyPortalTrigger(){
    DestroyTriggerEventWithTrigger(131);
}

void HitPortal_DestroyButtonTrigger(){
    DestroyTriggerEventWithTrigger(130);
}

void ActivatePortal(unsigned int triggerID){
    currentDimension = triggerID - 13;
    dimensionTimer = 15;
}

void ActivateButtonInDimension(unsigned int triggerID){
    if(currentDimension == 1){
        ActivateButton(triggerID);
        DestroyTriggerEventWithTrigger(triggerID);
    }
}

void LoadNextLevel(){
    TraceLog(LOG_INFO, "Attemping to load next level...");
    currentDimension = 0;
    if(levelSelect != 0) UnloadTexture(levelBackground);
    levelBackground = LoadTexture(TextFormat("./../res/Levels/grayzone_level%d.png", levelSelect + 1));

    int bodyCount = GetPhysicsBodiesCount();
    for(int i = 0; i < bodyCount - 2; i++){
        DestroyPhysicsBody(GetPhysicsBody(2));
    }
    ClearDisplayTextQueue();
    ClearDisplayText();

    DestroyAllLevelObjects();

    ResetAllTriggers();
    
    // Pre defined TriggerEvents
    NewTriggerEvent(1, TRIGGER_USE_ON_STAY, CreateTriggerEventFunctionData_SetForce(AddPlayerInputForce));
    NewTriggerEvent(3, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_NoArgFunction(LoadNextLevel));
    
    NewTriggerEvent(4, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_NoArgFunction(StartDeathAnimation));

    NewTriggerEvent(13, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_FunctionWithTriggerID(ActivatePortal));
    NewTriggerEvent(14, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_FunctionWithTriggerID(ActivatePortal));
    NewTriggerEvent(15, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_FunctionWithTriggerID(ActivatePortal));
    NewTriggerEvent(16, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_FunctionWithTriggerID(ActivatePortal));

    NewTriggerEvent(17, TRIGGER_USE_ON_ENTER, CreateTriggerEventFunctionData_FunctionWithTriggerID(ActivateButtonInDimension));
    
    NewTriggerEvent(130, TRIGGER_USE_ONCE, CreateTriggerEventFunctionData_NoArgFunction(HitButton_DestroyPortalTrigger));
    NewTriggerEvent(131, TRIGGER_USE_ONCE, CreateTriggerEventFunctionData_NoArgFunction(HitPortal_DestroyButtonTrigger));

    parseStructGroupInfo(readFileSF(TextFormat("./../res/LevelFiles/%d.sf", levelSelect + 1)), DrawHaroldText, &startingPos);

    player->position = startingPos;

    levelSelect++;
}
