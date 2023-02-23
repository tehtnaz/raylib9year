#pragma once

#include "raylib.h"
#include "animation.h"

typedef struct Wire{
    Vector2 pos;
    unsigned int wireID;
    unsigned int trigger; // id of trigger attached to it
    bool on;
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

#define MAX_BUTTONS 8
#define MAX_PORTALS 4
#define MAX_WIRES 2
#define WIRE_ID_COUNT 7

Animation* portalIDToAnimation(int portalID);

void LevelObjectsInit();

void ActivateButton(unsigned int triggerID);

void RenderLevelObjects();

Wire* CreateWire(Vector2 pos, unsigned int wireID, unsigned int trigger, bool on);
Button* CreateButton(Vector2 pos, unsigned int trigger, bool buttonDown);
Portal* CreatePortal(Vector2 pos, unsigned int colourID);

void DestroyAllLevelObjects();