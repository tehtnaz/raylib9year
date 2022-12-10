#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
//#include "levelStructs.h"
#include "raylib.h"

#pragma once

//  dataHandling v4
//      v3: proper tokenization, complete rewrite, v2 completely thrown out
//      v4: parse split into different function, made more portable

typedef enum TOKEN_TYPE{
    //void + error types
    UNDEFINED,
    NO_TYPE,

    //basic types (put any enums here)
        TRUE, FALSE,
        INTEGER, FLOAT,
        STRING,
        ITEMTYPE, //(enum)

    //modifier
        MINUS,

    //groups
    LEFT_PAREN,
    RIGHT_PAREN,

    //your custom structs here (remember to add to readKeywords)
        //complex types
        VECTOR2,
        RECTANGLE,

        //root structs
        CIRCLE_PHYSOBJ,
        RECTANGLE_PHYSOBJ,
        TEXT_TRIGGER

}TOKEN_TYPE;

typedef struct TokenInfo{
    union{
        char* text;
        int integer;
        float decimal;
    };
    TOKEN_TYPE type;
    int line; 
    struct TokenInfo* next;
}TokenInfo;

typedef struct StructGroup{
    TokenInfo token;
    struct StructGroup* child;
    struct StructGroup* parent;
    struct StructGroup* next;
    struct StructGroup* previous;
}StructGroup;

typedef struct TempPhysObj{
    Vector2 pos;
    float radius;   //for circles
    float width;    // for recs
    float height;   // for recs
    bool isStatic;
    unsigned int tag;
    unsigned int trigger;
} TempPhysObj;

typedef struct TextBoxTrigger{
    int trigger;
    char* text;
}TextBoxTrigger;

StructGroup* readFileSF(const char* path);

#ifdef _DEBUG
#define DEBUG_DATA_HANDLING
#endif

int parseStructGroupInfo(StructGroup* groupRoot, void (*function_harold_prompt)(const char* text));