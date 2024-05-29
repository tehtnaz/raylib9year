#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "physac.h"
#include "triggers.h"

static TriggerEvent triggerEventArray[MAX_TRIGGER_EVENT_COUNT];
static int triggerEventCount = 0;

static int triggerSelector = 0;

//define this so we can keep at bottom but still use
void DestroyTriggerEventAtIndex(int index);


TriggerEventFunctionData CreateTriggerEventFunctionData_WithOriginBody(void (*function_add_force)(PhysicsBody body)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_WITH_ORIGIN_BODY;
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
TriggerEventFunctionData CreateTriggerEventFunctionData_NoArgFunction(void (*function_no_arg_function)()){
    TriggerEventFunctionData data;
    data.type = TRIGGER_FUNCTION;
    data.function_no_arg_function = function_no_arg_function;
    return data;
}
TriggerEventFunctionData CreateTriggerEventFunctionData_FunctionWithTriggerID(void (*function_with_trigger_function)(unsigned int triggerID)){
    TriggerEventFunctionData data;
    data.type = TRIGGER_FUNCTION_WITH_TRIGGERID;
    data.function_with_trigger_function = function_with_trigger_function;
    return data;
}

void NewTriggerEvent(unsigned int triggerID, TriggerUseType useType, TriggerEventFunctionData data){
    if(triggerEventCount == MAX_TRIGGER_EVENT_COUNT){
        TraceLog(LOG_ERROR, "Triggers - Could not create more trigger events (max reached)");
        return;
    }
    TriggerEvent event;
    event.triggerID = triggerID;
    event.wasUsedOnPreviousFrame = false;
    event.inUse = false;
    event.bodyOrigin = NULL;
    event.useType = useType;
    event.data = data;

    #ifdef _DEBUG
        TraceLog(LOG_DEBUG, "Triggers - Creating new trigger at index %d with trigger %d and type %d", triggerEventCount, event.triggerID, event.data.type);
    #endif

    triggerEventArray[triggerEventCount] = event;
    triggerEventCount++;
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

void ActivateTriggerEvent(int index){
    #ifdef _DEBUG
        TraceLog(LOG_DEBUG, "Triggers - Activating index %d with trigger %d and type %d and useType %d", index, triggerEventArray[index].triggerID, triggerEventArray[index].data.type, triggerEventArray[index].useType);
        TraceLog(LOG_DEBUG, "triggerEventCount: %d", triggerEventCount);
    #endif
    TriggerEventFunctionData data = triggerEventArray[index].data;
    switch (data.type){
        case TRIGGER_FUNCTION: data.function_no_arg_function(); break;
        case TRIGGER_TEXT_PROMPT: data.function_text_prompt((const char**)data.texts, data.textCount); break;
        case TRIGGER_FUNCTION_WITH_TRIGGERID: data.function_with_trigger_function(triggerEventArray[index].triggerID); break;
        case TRIGGER_WITH_ORIGIN_BODY: 
            if(triggerEventArray[index].bodyOrigin == NULL) {
                TraceLog(LOG_ERROR, "Triggers - Body origin was NULL while trying to a function using it!");
            } else data.function_add_force(triggerEventArray[index].bodyOrigin); 
            break;
        default: TraceLog(LOG_ERROR, "Triggers - Invalid dataType");
    }
}
void ActivateAllTriggerInUse(){
    triggerSelector = 0;
    while(triggerSelector < triggerEventCount){
        //TraceLog("check %d; ", i);

        if(triggerEventArray[triggerSelector].useType == TRIGGER_USE_ONCE && triggerEventArray[triggerSelector].inUse){
            ActivateTriggerEvent(triggerSelector);
            DestroyTriggerEventAtIndex(triggerSelector);
        }else if(
            (triggerEventArray[triggerSelector].useType == TRIGGER_USE_ON_ENTER &&  triggerEventArray[triggerSelector].inUse && !triggerEventArray[triggerSelector].wasUsedOnPreviousFrame) ||
            (triggerEventArray[triggerSelector].useType == TRIGGER_USE_ON_EXIT  && !triggerEventArray[triggerSelector].inUse &&  triggerEventArray[triggerSelector].wasUsedOnPreviousFrame) ||
            (triggerEventArray[triggerSelector].useType == TRIGGER_USE_ON_STAY  &&  triggerEventArray[triggerSelector].inUse)
        ){
            ActivateTriggerEvent(triggerSelector);
        }

        triggerSelector++;
    }
    //TraceLog("thing what: %d", thing);
}

void SetTriggerInUse(PhysicsBody body, int triggerID){
    for(int i = 0; i < triggerEventCount; i++){
        if(triggerEventArray[i].triggerID != triggerID){
            continue;
        }
        #ifdef _DEBUG
            //TraceLog(LOG_DEBUG, "Triggers - Now in use event with index %d with trigger %d and type %d", i, triggerID, triggerEventArray[i].data.type);
        #endif
        triggerEventArray[i].inUse = true;
        triggerEventArray[i].bodyOrigin = body;
    }
}

void UpdateAndActivateTriggers(){
    const int manifoldCount = GetPhysicsManifoldCount();
    //TraceLog("");
    // Refresh all values
    for(int i = 0; i < triggerEventCount; i++){
        triggerEventArray[i].wasUsedOnPreviousFrame = triggerEventArray[i].inUse;
        triggerEventArray[i].inUse = false;
        triggerEventArray[i].bodyOrigin = NULL;
        if(triggerEventArray[i].useType == TRIGGER_USE_ONCE && triggerEventArray[i].wasUsedOnPreviousFrame){
            TraceLog(LOG_DEBUG, "Triggers - Cleaning up used trigger...");
            DestroyTriggerEventAtIndex(i);
            i--;
        }
    }

    // Check which items are contacting eachother
    for(int i = 0; i < manifoldCount; i++){
        //TraceLog("aID: %d bID: %d cc: %d| ", contacts[i]->bodyA->id, contacts[i]->bodyB->id, contacts[i]->contactsCount);
        PhysicsManifold manifold = GetPhysicsManifold(i);
        if(manifold->contactsCount > 0){
            if(manifold->bodyA->tagCount > PHYSAC_MAX_TAG_COUNT || manifold->bodyB->tagCount > PHYSAC_MAX_TAG_COUNT){
                TraceLog(LOG_ERROR, "Triggers - Max tagCount exceeded");
                return;
            }
            for(int i = 0; i < manifold->bodyA->tagCount; i++){
                if(manifold->bodyA->tags[i] == manifold->bodyB->trigger
                    || (manifold->bodyA->tags[i] == 2 && manifold->bodyB->trigger > 8)){
                    SetTriggerInUse(manifold->bodyA, manifold->bodyB->trigger);
                }
            }
            for(int i = 0; i < manifold->bodyB->tagCount; i++){
                if(manifold->bodyB->tags[i] == manifold->bodyA->trigger
                    || (manifold->bodyB->tags[i] == 2 && manifold->bodyA->trigger > 8)){
                    SetTriggerInUse(manifold->bodyB, manifold->bodyA->trigger);
                }
            }
        }
    }
    ActivateAllTriggerInUse();
}


void DestroyTriggerEventWithTrigger(int triggerID){
    TraceLog(LOG_DEBUG, "Triggers - Previous triggerEventCount: %d", triggerEventCount);
    for(int i = 0; i < triggerEventCount; i++){
        if(triggerEventArray[i].triggerID == triggerID){
            TraceLog(LOG_DEBUG, "Triggers - Destroying TriggerEvent with trigger %d", triggerID);
            ClearTriggerEventFunctionData(triggerEventArray[i]);
            for(int j = i; j < triggerEventCount - 1; j++){
                triggerEventArray[j] = triggerEventArray[j + 1];
            }
            triggerEventCount--;
            if(triggerSelector >= i) triggerSelector--;
        }
    }
    TraceLog(LOG_DEBUG, "Triggers - New triggerEventCount: %d", triggerEventCount);
}
void DestroyTriggerEventAtIndex(int index){
    TraceLog(LOG_DEBUG, "Triggers - Destroying TriggerEvent at index %d with trigger %d and type %d", index, triggerEventArray[index].triggerID, triggerEventArray[index].data.type);
    ClearTriggerEventFunctionData(triggerEventArray[index]);
    for(int j = index; j < triggerEventCount - 1; j++){
        triggerEventArray[j] = triggerEventArray[j + 1];
    }
    triggerEventCount--;
    if(triggerSelector >= index) triggerSelector--;
}