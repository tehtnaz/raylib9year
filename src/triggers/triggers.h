#ifndef TRIGGERS_H
#define TRIGGERS_H

#include "physac.h"

#define MAX_TRIGGER_EVENT_COUNT 64

//use if we actually use non-static functions lol
typedef enum TriggerType{
    TRIGGER_FUNCTION,
    TRIGGER_SET_FORCE,
    TRIGGER_TEXT_PROMPT,
    TRIGGER_FUNCTION_WITH_TRIGGERID

}TriggerType;

//typedef struct TriggerEventData *TriggerEvent;

typedef struct TriggerEventFunctionData{
    TriggerType type;

    void (*function_no_arg_function)();

    void (*function_add_force)(PhysicsBody body);

    char** texts;
    int textCount;
    void (*function_text_prompt)(const char** texts, int textCount);

    void (*function_with_trigger_function)(unsigned int triggerID);

}TriggerEventFunctionData;

typedef enum TriggerUseType{
    TRIGGER_USE_ONCE,
    TRIGGER_USE_ON_ENTER,
    TRIGGER_USE_ON_EXIT,
    TRIGGER_USE_ON_STAY
}TriggerUseType;

typedef struct TriggerEvent{
    unsigned int triggerID;
    bool wasUsedOnPreviousFrame;
    bool inUse;
    PhysicsBody bodyOrigin;
    TriggerUseType useType;

    TriggerEventFunctionData data;
}TriggerEvent;

TriggerEventFunctionData CreateTriggerEventFunctionData_SetForce(void (*function_add_force)(PhysicsBody body));
TriggerEventFunctionData CreateTriggerEventFunctionData_TextPrompt(const char** texts, int textCount, void (*function_text_prompt)(const char** texts, int textCount));
TriggerEventFunctionData CreateTriggerEventFunctionData_NoArgFunction(void (*function_no_arg_function)());
TriggerEventFunctionData CreateTriggerEventFunctionData_FunctionWithTriggerID(void (*function_with_trigger_function)(unsigned int triggerID));

void NewTriggerEvent(unsigned int triggerID, TriggerUseType useType, TriggerEventFunctionData data);

void ResetAllTriggers();

void UpdateAndActivateTriggers();

void DestroyTriggerEventWithTrigger(int triggerID);

#endif