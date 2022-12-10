#pragma once

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "physac.h"

//use if we actually use non-static functions lol
typedef enum TriggerType{
    TRIGGER_FUNCTION,
    TRIGGER_SET_FORCE,
    TRIGGER_TEXT_PROMPT,

}TriggerType;

//typedef struct TriggerEventData *TriggerEvent;

typedef struct TriggerEventFunctionData{
    TriggerType type;

    void (*function_function)();

    void (*function_add_force)(PhysicsBody body);

    char* text;
    void (*function_text_prompt)(const char* text);

}TriggerEventFunctionData;

typedef struct TriggerEvent{
    unsigned int triggerID;
    bool oneTimeUse;
    bool wasUsed;

    TriggerEventFunctionData data;
}TriggerEvent;

#define MAX_TRIGGER_COUNT 8

static TriggerEvent triggerEventArray[MAX_TRIGGER_COUNT];
static int triggerEventCount = 0;

TriggerEventFunctionData CreateTriggerEventFunctionData_SetForce(void (*function_add_force)(PhysicsBody body)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_SET_FORCE;
    data.function_add_force = function_add_force;
    return data;
}
TriggerEventFunctionData CreateTriggerEventFunctionData_TextPrompt(const char* text, void (*function_text_prompt)(const char* text)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_TEXT_PROMPT;
    data.function_text_prompt = function_text_prompt;
    data.text = calloc(TextLength(text), sizeof(char));
    TextCopy(data.text, text);
    return data;
}
TriggerEventFunctionData CreateTriggerEventFunctionData_Function(void (*function_function)()){
    TriggerEventFunctionData data;
    data.type = TRIGGER_FUNCTION;
    data.function_function = function_function;
    return data;
}

void NewTriggerEvent(unsigned int triggerID, bool oneTimeUse, TriggerEventFunctionData data){
    if(triggerEventCount == MAX_TRIGGER_COUNT){
        printf("TRIGGER_H: Error - Could not create more trigger events (max reached)\n");
        return;
    }
    TriggerEvent event;
    event.triggerID = triggerID;
    event.oneTimeUse = oneTimeUse;
    event.wasUsed = false;
    event.data = data;

    triggerEventArray[triggerEventCount] = event;
    triggerEventCount++;
}

void ActivateTrigger(PhysicsBody body, int triggerID){
    int i = 0;
    for(int j = 0; j < triggerEventCount; j++){
        if(triggerEventArray[j].triggerID == triggerID) {
            i = j;
            break;
        }
    }

    if(triggerEventArray[i].oneTimeUse && triggerEventArray[i].wasUsed){
        #ifdef _DEBUG
        printf("TRIGGER_H: Debug - Tried activating already activated one time use trigger\r");
        #endif
        return;
    }
    triggerEventArray[i].wasUsed = true;
    TriggerEventFunctionData data = triggerEventArray[i].data;
    //printf("lets make love %p a\n", data.function_text_prompt);
    switch (data.type){
        case TRIGGER_FUNCTION: data.function_function(); break;
        case TRIGGER_SET_FORCE: data.function_add_force(body); break;
        case TRIGGER_TEXT_PROMPT: data.function_text_prompt(data.text); break;
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