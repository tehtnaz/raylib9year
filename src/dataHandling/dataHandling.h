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
        COLLIDER2D

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

StructGroup* readFileSF(const char* path);

int parseStructGroupInfo(StructGroup* groupRoot);