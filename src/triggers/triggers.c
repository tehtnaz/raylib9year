#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "physac.h"
#include "triggers.h"

#define MAX_TRIGGER_COUNT 32

static TriggerEvent triggerEventArray[MAX_TRIGGER_COUNT];
static int triggerEventCount = 0;

TriggerEventFunctionData CreateTriggerEventFunctionData_SetForce(void (*function_add_force)(PhysicsBody body)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_SET_FORCE;
    data.function_add_force = function_add_force;
    return data;
}
TriggerEventFunctionData CreateTriggerEventFunctionData_TextPrompt(const char** texts, int textCount, void (*function_text_prompt)(const char** texts, int textCount)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_TEXT_PROMPT;
    data.function_text_prompt = function_text_prompt;
    data.texts = malloc(textCount * sizeof(char*));
    for(int i = 0; i < textCount; i++){
        data.texts[i] = calloc(TextLength(texts[i]), sizeof(char));
        TextCopy(data.texts[i], texts[i]);
    }
    data.textCount = textCount;
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
        case TRIGGER_TEXT_PROMPT: data.function_text_prompt(data.texts, data.textCount); break;
        default: printf("Invalid dataType\n");
    }

}

void ActivateAllContactedTriggers(){
    const int manifoldCount = GetPhysicsManifoldCount();
    for(int i = 0; i < manifoldCount; i++){
        //printf("aID: %d bID: %d cc: %d| ", contacts[i]->bodyA->id, contacts[i]->bodyB->id, contacts[i]->contactsCount);
        PhysicsManifold manifold = GetPhysicsManifold(i);
        if(manifold->contactsCount > 0){
            for(int i = 0; i < manifold->bodyA->tagCount; i++){
                if(manifold->bodyA->tags[i] == manifold->bodyB->trigger){
                    ActivateTrigger(manifold->bodyA, manifold->bodyB->trigger);
                }
            }

            for(int i = 0; i < manifold->bodyB->tagCount; i++){
                if(manifold->bodyB->tags[i] == manifold->bodyA->trigger){
                    ActivateTrigger(manifold->bodyB, manifold->bodyA->trigger);
                }
            }
        }
    }
}

void ClearTriggerEventFunctionData(TriggerEvent event){
    if(event.data.type == TRIGGER_TEXT_PROMPT){
        for(int i = 0; i < event.data.textCount; i++){
            free(event.data.texts[i]);
        }
        free(event.data.texts);
    }
    event.data = (TriggerEventFunctionData){0};
}

void ResetAllTriggers(){
    for(int i = 0; i < triggerEventCount; i++){
        ClearTriggerEventFunctionData(triggerEventArray[i]);
        triggerEventArray[i] = (TriggerEvent){};
    }
    triggerEventCount = 0;
}