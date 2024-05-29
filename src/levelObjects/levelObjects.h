#pragma once

#include "raylib.h"
#include "animation.h"
#include "physac.h"

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
    int doorType;
    Vector2 pos;
    Animation anim;
    unsigned int triggerID;
    PhysicsBody physObj;
    int isChangingState; // if = -1 -> moving backwards/closing | if = 1 -> moving forwards/opening | if = 0 -> not moving
}Door;

typedef struct LevelObjectFileData{
    Vector2 pos;
    unsigned int specialID; // can be either a doorType or colourID
    unsigned int trigger;
}LevelObjectFileData;

typedef struct PhysObjFileData{
    Vector2 pos;    // position at top left for rects, centre for circles
    float radius;   //for circles
    float width;    // for recs
    float height;   // for recs
    bool isStatic;
    unsigned int* tags;
    int tagCount;
    unsigned int trigger;
} PhysObjFileData;

#define MAX_BUTTONS 8
#define MAX_PORTALS 16
#define MAX_WIRES 2
#define MAX_DOORS 4
#define WIRE_ID_COUNT 7
#define MAX_CRATES 2
#define MAX_PORTAL_LOCATIONS 12

Animation* portalIDToAnimation(int portalID);

void LevelObjectsInit();

void ActivateButton(unsigned int triggerID);
void ActivateDoor(unsigned int triggerID);

void UpdateDoors();

void RenderLevelObjects();

// Wire* CreateWire(Vector2 pos, unsigned int wireID, unsigned int trigger, bool isOn);
// Button* CreateButton(Vector2 pos, unsigned int trigger, bool buttonDown);
// Portal* CreatePortal(Vector2 pos, unsigned int colourID);
// Door* CreateDoor(Vector2 pos, int doorType, unsigned int trigger);
void CreateWireFromData(LevelObjectFileData data);
void CreateButtonFromData(LevelObjectFileData data);
void CreatePortalFromData(LevelObjectFileData data);
void CreateDoorFromData(LevelObjectFileData data);
void AssignPortalLocationFromData(LevelObjectFileData data);

void CreatePhysObjFromData(PhysObjFileData data, bool isCircle);

void CreateCrate(Vector2 pos);

Vector2 GetPortalLocation(int triggerId);

void DestroyAllLevelObjects();