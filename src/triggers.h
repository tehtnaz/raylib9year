#pragma once

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "physac.h"

//use if we actually use non-static functions lol
typedef enum TriggerType{
    TRIGGER_FUNCTION,
    TRIGGER_SET_FORCE,
    TRIGGER_SOUND,

}TriggerType;

typedef struct TriggerEventData *TriggerEvent;

typedef struct TriggerEventFunctionData{
    TriggerType type;

    void (*function_add_force)(PhysicsBody body);

}TriggerEventFunctionData;

typedef struct TriggerEventData{
    unsigned int triggerID;
    bool oneTimeUse;
    bool wasUsed;

    TriggerEventFunctionData data;
}TriggerEventData;

#define MAX_TRIGGER_COUNT 8

static TriggerEvent triggerEventArray[MAX_TRIGGER_COUNT];
static int triggerEventCount = 0;

TriggerEventFunctionData CreateTriggerEventFunctionData_SetForce(void (*function_add_force)(PhysicsBody body)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_SET_FORCE;
    data.function_add_force = function_add_force;
    return data;
}

void NewTriggerEvent(unsigned int triggerID, bool oneTimeUse, TriggerEventFunctionData data){
    if(triggerEventCount == MAX_TRIGGER_COUNT){
        printf("TRIGGER_H: Error - Could not create more trigger events (max reached)\n");
        return;
    }
    TriggerEvent event = calloc(sizeof(TriggerEvent), 1);
    event->triggerID = triggerID;
    event->oneTimeUse = oneTimeUse;
    event->wasUsed = false;
    event->data = data;

    triggerEventArray[triggerEventCount] = event;
    triggerEventCount++;
}

void ActivateTrigger(PhysicsBody body, int triggerID){
    int i = 0;
    for(int j = 0; j < triggerEventCount; j++){
        if(triggerEventArray[j]->triggerID == triggerID) {
            i = j;
            break;
        }
    }

    if(triggerEventArray[i]->oneTimeUse && triggerEventArray[i]->wasUsed){
        #ifdef _DEBUG
        printf("TRIGGER_H: Debug - Tried activating already activated one time use trigger\n");
        #endif
        return;
    }
    triggerEventArray[i]->wasUsed = true;
    TriggerEventFunctionData data = triggerEventArray[i]->data;
    
    switch (data.type){
        case TRIGGER_SET_FORCE: data.function_add_force(body); break;
        default: printf("Invalid dataType\n");
    }

}

void ActivateAllContactedTriggers(){
    const int manifoldCount = GetPhysicsManifoldCount();
    for(int i = 0; i < manifoldCount; i++){
        //printf("aID: %d bID: %d cc: %d| ", contacts[i]->bodyA->id, contacts[i]->bodyB->id, contacts[i]->contactsCount);
        PhysicsManifold manifold = GetPhysicsManifold(i);
        if(manifold->contactsCount > 0){
            if(manifold->bodyA->tag == manifold->bodyB->trigger){
                ActivateTrigger(manifold->bodyA, manifold->bodyB->trigger);
            }

            if(manifold->bodyB->tag == manifold->bodyA->trigger){
                ActivateTrigger(manifold->bodyB, manifold->bodyA->trigger);
            }
        }
    }
}