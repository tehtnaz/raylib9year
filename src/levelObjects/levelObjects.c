#include "levelObjects.h"
#include "animatedColliders.h"
#include <stdlib.h>
#include <stdio.h>

// all draw calls have CYCLE_NONE
// cycling is done at end of frame
static int portalCount;
static Portal portalArray[MAX_PORTALS];
static Animation portalRed;
static Animation portalBlue;
static Animation portalGreen;
static Animation portalYellow;

static int buttonCount;
static Button buttonArray[MAX_BUTTONS];
static Texture2D buttonDown;
static Texture2D buttonUp;

static int wireCount;
static Wire wireArray[MAX_WIRES];
static Texture2D wireOn[WIRE_ID_COUNT];
static Texture2D wireOff[WIRE_ID_COUNT];

static int doorCount;
static Door doorArray[MAX_DOORS];
static Texture2D normalDoor;
static Texture2D pistonDoor;
static Texture2D trapDoor;

static Texture2D crateTexture;
static int crateIndexArray[MAX_CRATES]; //PhysicsBody index of all crates
static int crateCount;

Animation* portalIDToAnimation(int portalID){
    switch(portalID){
        case 1:
            return &portalRed;
        case 2:
            return &portalBlue;
        case 3:
            return &portalGreen;
        case 4:
            return &portalYellow;
        default:
            TraceLog(LOG_WARNING, "levelObjects - Unknown portalId received; id = %d", portalID);
            return &portalRed;
    }
}

Vector2 GetPhysicsBodyTexturePosition(int index, float textureWidth, float textureHeight){
    Vector2 tempPos = GetPhysicsBody(index)->position;
    tempPos.x -= textureWidth / 2.0f;
    tempPos.y -= textureHeight / 2.0f;
    return tempPos;
}

void LevelObjectsInit(){
    buttonDown = LoadTexture("./../res/Buttons/button_red.png");
    buttonUp = LoadTexture("./../res/Buttons/button_green.png");

    portalRed = assignProperties(0, 0, 6, true, 3, false);
    portalRed = GetAnimationFromFolder(portalRed, true, "./../res/Portals/red/");
    portalBlue = assignProperties(0, 0, 6, true, 3, false);
    portalBlue = GetAnimationFromFolder(portalBlue, true, "./../res/Portals/blue/");
    portalGreen = assignProperties(0, 0, 6, true, 3, false);
    portalGreen = GetAnimationFromFolder(portalGreen, true, "./../res/Portals/green/");
    portalYellow = assignProperties(0, 0, 6, true, 3, false);
    portalYellow = GetAnimationFromFolder(portalYellow, true, "./../res/Portals/yellow/");

    normalDoor = GetTextureAtlasFromFolder("./../res/Doors/red_door_animation/", 3);
    pistonDoor = GetTextureAtlasFromFolder("./../res/Doors/red_push_door_animation/", 67);
    trapDoor = GetTextureAtlasFromFolder("./../res/Doors/red_trap_door_animation/", 19);

    crateTexture = LoadTexture("./../res/Objects/movable_crate.png");

    for(int i = 0; i < WIRE_ID_COUNT; i++){
        wireOn[i] = LoadTexture(TextFormat("./../res/Wires/on/%d.png", i));
        wireOff[i] = LoadTexture(TextFormat("./../res/Wires/off/%d.png", i));
    }

    TraceLog(LOG_INFO, "levelObjects - Initialized.");
}

void ActivateButton(unsigned int triggerID){
    TraceLog(LOG_DEBUG, "ActivateButton - activating triggerID %d", triggerID);
    for(int i = 0; i < buttonCount; i++){
        if(buttonArray[i].trigger == triggerID){
            buttonArray[i].buttonDown = true;
        }
    }
    for(int i = 0; i < wireCount; i++){
        if(wireArray[i].trigger == triggerID){
            wireArray[i].isOn = true;
        }
    }
    ActivateDoor(triggerID);
    // for(int i = GetPhysicsBodiesCount() - 1; i >= 0; i--){
    //     if(GetPhysicsBody(i)->trigger == )
    // }
}

void ActivateDoor(unsigned int triggerID){
    TraceLog(LOG_DEBUG, "ActivateDoor - activating triggerID %d", triggerID);
    for(int i = 0; i < doorCount; i++){
        if(doorArray[i].triggerID == triggerID){
            doorArray[i].isChangingState = 1;
            doorArray[i].anim.isAnimating = true;
            doorArray[i].physObj->trigger = 5;
            TraceLog(LOG_DEBUG, "ActivateDoor - activated door index - %d", i);
        }
    }
}

void UpdateDoors(){
    for(int i = 0; i < doorCount; i++){
        if(doorArray[i].isChangingState != 0){
            //printf("pointer to anim %p\n", &doorArray[i].anim);
            if(doorArray[i].isChangingState == 1) CycleAnimation(&doorArray[i].anim);
            else CycleAnimationBackwards(&doorArray[i].anim);
            if(doorArray[i].anim.isAnimating == false) doorArray[i].isChangingState = 0;
        }
    }
}

void RenderLevelObjects(){
    for(int i = 0; i < buttonCount; i++){
        DrawTextureEx(buttonArray[i].buttonDown ? buttonDown : buttonUp, buttonArray[i].pos, 0, 1, WHITE);
    }

    for(int i = 0; i < portalCount; i++){
        DrawAnimationPro(portalIDToAnimation(portalArray[i].colourID), portalArray[i].pos, 1, WHITE, CYCLE_NONE);
    }

    for(int i = 0; i < wireCount; i++){
        DrawTextureEx(wireArray[i].isOn ? wireOn[wireArray[i].wireID] : wireOff[wireArray[i].wireID], wireArray[i].pos, 0, 1, WHITE);
    }

    for(int i = 0; i < doorCount; i++){
        DrawAnimationPro(&doorArray[i].anim, doorArray[i].pos, 1, WHITE, CYCLE_NONE);
    }

    for(int i = 0; i < crateCount; i++){
        DrawTextureEx(crateTexture, GetPhysicsBodyTexturePosition(crateIndexArray[i], crateTexture.width, crateTexture.height), 0, 1, WHITE);
    }

    ShakeCycleAnimation(&portalRed);
    ShakeCycleAnimation(&portalBlue);
    ShakeCycleAnimation(&portalGreen);
    ShakeCycleAnimation(&portalYellow);
}

Wire* CreateWire(Vector2 pos, unsigned int wireID, unsigned int trigger, bool isOn){
    if(wireCount == MAX_WIRES){
        TraceLog(LOG_WARNING, "levelObjects - MAX_WIRES limit reached. Couldn't create new wire");
        return NULL;
    }
    wireArray[wireCount] = (Wire){pos, wireID, trigger, isOn};
    wireCount++;

    return &(wireArray[wireCount - 1]);
}
Button* CreateButton(Vector2 pos, unsigned int trigger, bool m_buttonDown){
    if(buttonCount == MAX_BUTTONS){
        TraceLog(LOG_WARNING, "levelObjects - MAX_BUTTONS limit reached. Couldn't create new button");
        return NULL;
    }
    PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){pos.x + 15, pos.y + 5.5f}, 30, 11, 1, trigger, 0);
    body->freezeOrient = true;
    body->enabled = false;

    buttonArray[buttonCount] = (Button){pos, trigger, m_buttonDown};
    buttonCount++;

    return &(buttonArray[buttonCount - 1]);
}
Portal* CreatePortal(Vector2 pos, unsigned int colourID){
    if(portalCount == MAX_PORTALS){
        TraceLog(LOG_WARNING, "levelObjects - MAX_PORTALS limit reached. Couldn't create new portal");
        return NULL;
    }
    PhysicsBody body = CreatePhysicsBodyCircle((Vector2){pos.x + 13, pos.y + 22}, 14, 1, colourID + 12, 0);
    body->freezeOrient = true;
    body->enabled = false;

    portalArray[portalCount] = (Portal){pos, colourID};
    portalCount++;

    return &(portalArray[portalCount - 1]);
}

//(0 not used so that they can be detected as errors if null value is loaded from file)
//doorType, 1 = normal, 2 = pushdoor, 3 = trapdoor 
Door* CreateDoor(Vector2 pos, int doorType, unsigned int trigger){
    if(doorCount == MAX_DOORS){
        TraceLog(LOG_WARNING, "levelObjects - MAX_DOORS limit reached. Couldn't create new door");
        return NULL;
    }
    Animation tempAnim = assignProperties(0, 0, 0, false, 0, false);
    switch (doorType)
    {
        case 1:
            tempAnim.texture = normalDoor;
            tempAnim.frameCount = 3;
            tempAnim.spriteWidth = normalDoor.width / tempAnim.frameCount;
            tempAnim.fps = 2;
            break;
        case 2:
            tempAnim.texture = pistonDoor;
            tempAnim.frameCount = 67;
            tempAnim.spriteWidth = pistonDoor.width / tempAnim.frameCount;
            tempAnim.fps = 15;
            break;
        case 3:
            tempAnim.texture = trapDoor;
            tempAnim.frameCount = 19;
            tempAnim.spriteWidth = trapDoor.width / tempAnim.frameCount;
            tempAnim.fps = 9;
            break;
        default:
            TraceLog(LOG_WARNING, "levelObjects - Received incorrect doorType value (value was %d)", doorType);
            return NULL;
    }
    PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){pos.x + tempAnim.spriteWidth / 2, pos.y + (doorType == 2 ? 16 : tempAnim.texture.height / 2)}, tempAnim.spriteWidth, doorType == 2 ? 32 : tempAnim.texture.height, 1, 0, doorType == 2 ? 1 : 0);
    body->enabled = (doorType == 2);
    body->freezeOrient = true;

    doorArray[doorCount] = (Door){doorType, pos, tempAnim, trigger, body, 0};
    doorCount++;

    if(doorType == 2){
        OverridePhysicsBodyTriggerDynamics(body, true);
        
        PhysicsBody firstMovingBody = CreatePhysicsBodyRectangle((Vector2){pos.x + tempAnim.spriteWidth / 2, pos.y + 16}, tempAnim.spriteWidth, 32, 1, 0, 1);
        firstMovingBody->enabled = true;
        firstMovingBody->freezeOrient = true;
        OverridePhysicsBodyTriggerDynamics(firstMovingBody, true);
        
        PhysicsBody secondMovingBody = CreatePhysicsBodyRectangle((Vector2){pos.x + tempAnim.spriteWidth / 2, pos.y + 16}, tempAnim.spriteWidth, 32, 1, 0, 1);
        AddTagToPhysicsBody(secondMovingBody, 6);
        secondMovingBody->enabled = true;
        secondMovingBody->freezeOrient = true;
        OverridePhysicsBodyTriggerDynamics(secondMovingBody, true);

        CreateAnimatedCollider(body->position, (Vector2){body->position.x, body->position.y + 33}, firstMovingBody, &doorArray[doorCount - 1].anim);
        CreateAnimatedCollider(body->position, (Vector2){body->position.x, body->position.y + 66}, secondMovingBody, &doorArray[doorCount - 1].anim);

        TraceLog(LOG_DEBUG, "Creating new PistonDoor with props:\n-body-x:%f-y:%f\n-firstMoving-x:%f-y:%f\n-secondMoving-x:%f-y:%f", body->position.x, body->position.y, firstMovingBody->position.x, firstMovingBody->position.y, secondMovingBody->position.x, secondMovingBody->position.y);
    }    

    return &(doorArray[doorCount - 1]);
}

void CreateCrate(Vector2 pos){
    if(crateCount == MAX_CRATES){
        TraceLog(LOG_WARNING, "levelObjects - MAX_CRATES limit reached. Couldn't create new crate");
        return;
    }
    PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){pos.x + crateTexture.width / 2, pos.y + crateTexture.height / 2}, crateTexture.width, crateTexture.height, 20, 6, 0);
    OverridePhysicsBodyTriggerDynamics(body, true);
    body->enabled = true;
    body->freezeOrient = true;
    AddTagToPhysicsBody(body, 1);

    crateIndexArray[crateCount] = body->id;
    crateCount++;
}

// All physics bodies are destroyed at the end of each level, no need to do it here
void DestroyAllLevelObjects(){
    portalCount = 0;
    buttonCount = 0;
    wireCount = 0;
    doorCount = 0;
    crateCount = 0;
}

void CreateWireFromData(LevelObjectFileData data){CreateWire(data.pos, data.specialID, data.trigger, false);}
void CreateButtonFromData(LevelObjectFileData data){CreateButton(data.pos, data.trigger, false);};
void CreatePortalFromData(LevelObjectFileData data){CreatePortal(data.pos, data.specialID);};
void CreateDoorFromData(LevelObjectFileData data){CreateDoor(data.pos, data.specialID, data.trigger);};

void CreatePhysObjFromData(PhysObjFileData data, bool isCircle){
    PhysicsBody body = {0};
    if(isCircle) body = CreatePhysicsBodyCircle(data.pos, data.radius, 1, data.trigger, 0);
    else body = CreatePhysicsBodyRectangle((Vector2){data.pos.x + data.width / 2, data.pos.y + data.height / 2}, data.width, data.height, 1, data.trigger, 0);
    //both
    for(int i = 0; i < data.tagCount; i++){
        AddTagToPhysicsBody(body, data.tags[i]);
    }
    body->freezeOrient = true;
    body->enabled = !data.isStatic;
}