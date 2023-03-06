#pragma once

#include "raylib.h"
#include "animation.h"

typedef struct Wire{
    Vector2 pos;
    unsigned int wireID;
    unsigned int trigger; // id of trigger attached to it
    bool isOn;
}Wire;

typedef struct Button{
    Vector2 pos;
    unsigned int trigger; // id of trigger attached to it
    bool buttonDown;
    //timer to disable?
}Button;

typedef struct Portal{
    Vector2 pos;
    unsigned int colourID;
}Portal;

typedef struct Door{
    Vector2 pos;
    Animation anim;
    unsigned int triggerID;
    bool isOpen;
    bool isChangingState; // if isOpen = true, then it's closing; if isOpen = false, it's opening
}Door;

#define MAX_BUTTONS 8
#define MAX_PORTALS 4
#define MAX_WIRES 2
#define MAX_DOORS 4
#define WIRE_ID_COUNT 7

Animation* portalIDToAnimation(int portalID);

void LevelObjectsInit();

void ActivateButton(unsigned int triggerID);

void RenderLevelObjects();

Wire* CreateWire(Vector2 pos, unsigned int wireID, unsigned int trigger, bool isOn);
Button* CreateButton(Vector2 pos, unsigned int trigger, bool buttonDown);
Portal* CreatePortal(Vector2 pos, unsigned int colourID);
Door* CreateDoor(Vector2 pos, int doorType, unsigned int trigger);

void DestroyAllLevelObjects();