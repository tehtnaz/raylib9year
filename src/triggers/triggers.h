#ifndef TRIGGERS_H
#define TRIGGERS_H

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

    char** texts;
    int textCount;
    void (*function_text_prompt)(const char** texts, int textCount);

}TriggerEventFunctionData;

typedef struct TriggerEvent{
    unsigned int triggerID;
    bool oneTimeUse;
    bool wasUsed;

    TriggerEventFunctionData data;
}TriggerEvent;

TriggerEventFunctionData CreateTriggerEventFunctionData_SetForce(void (*function_add_force)(PhysicsBody body));
TriggerEventFunctionData CreateTriggerEventFunctionData_TextPrompt(const char** texts, int textCount, void (*function_text_prompt)(const char** texts, int textCount));
TriggerEventFunctionData CreateTriggerEventFunctionData_Function(void (*function_function)());

void NewTriggerEvent(unsigned int triggerID, bool oneTimeUse, TriggerEventFunctionData data);

void ActivateTrigger(PhysicsBody body, int triggerID);

void ResetAllTriggers();

void ActivateAllContactedTriggers();

#endif