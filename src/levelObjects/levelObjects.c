#include "levelObjects.h"
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
            printf("WARNING: levelObjects - Unknown portalId received; id = %d\n", portalID);
            return &portalRed;
    }
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

    for(int i = 0; i < WIRE_ID_COUNT; i++){
        wireOn[i] = LoadTexture(TextFormat("./../res/Wires/on/%d.png", i));
        wireOff[i] = LoadTexture(TextFormat("./../res/Wires/off/%d.png", i));
    }

    printf("levelObjects - Initialized.\n");
}

void ActivateButton(unsigned int triggerID){
    printf("DEBUG: ActiavateButton - activating id %d\n", triggerID);
    for(int i = 0; i < buttonCount; i++){
        if(buttonArray[i].trigger == triggerID){
            buttonArray[i].buttonDown = true;
        }
    }
    for(int i = 0; i < wireCount; i++){
        if(wireArray[i].trigger == triggerID){
            wireArray[i].on = true;
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
        DrawTextureEx(wireArray[i].on ? wireOn[wireArray->wireID] : wireOff[wireArray->wireID], wireArray[i].pos, 0, 1, WHITE);
    }

    ShakeCycleAnimation(&portalRed);
    ShakeCycleAnimation(&portalBlue);
    ShakeCycleAnimation(&portalGreen);
    ShakeCycleAnimation(&portalYellow);
}

Wire* CreateWire(Vector2 pos, unsigned int wireID, unsigned int trigger, bool on){
    if(wireCount == MAX_WIRES){
        printf("WARNING: levelObjects - MAX_WIRES limit reached. Couldn't create new wire");
        return NULL;
    }
    wireArray[wireCount] = (Wire){pos, wireID, trigger, on};
    wireCount++;

    return &(wireArray[wireCount - 1]);
}
Button* CreateButton(Vector2 pos, unsigned int trigger, bool m_buttonDown){
    if(buttonCount == MAX_BUTTONS){
        printf("WARNING: levelObjects - MAX_BUTTONS limit reached. Couldn't create new button");
        return NULL;
    }
    buttonArray[buttonCount] = (Button){pos, trigger, m_buttonDown};
    buttonCount++;

    return &(buttonArray[buttonCount - 1]);
}
Portal* CreatePortal(Vector2 pos, unsigned int colourID){
    if(portalCount == MAX_PORTALS){
        printf("WARNING: levelObjects - MAX_PORTALS limit reached. Couldn't create new portal");
        return NULL;
    }
    portalArray[portalCount] = (Portal){pos, colourID};
    portalCount++;

    return &(portalArray[portalCount - 1]);
}

void DestroyAllLevelObjects(){
    portalCount = 0;
    buttonCount = 0;
    wireCount = 0;
}