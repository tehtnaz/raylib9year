#pragma once

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

static char* cachedText = NULL;
static char* displayText = NULL;
static float maxTextWidth = 0;
static bool displayTextEnabled = false;
static Vector2 textPosition = {0, 0};
static float currentTimeBetween = 0;

#define MinTimeBetween 0.03f
#define DefaultFontSize 10.0f
#define DefaultLineSpacing 1.15f;
static const float spaceBetweenLines = DefaultFontSize * DefaultLineSpacing;
static const float spacing = DefaultFontSize / 10;

void UpdateAndDrawTypingText(Color color){
    if(!displayTextEnabled) return;

    int textLength = TextLength(displayText);

    currentTimeBetween += GetFrameTime();
    if(currentTimeBetween >= MinTimeBetween && textLength != TextLength(cachedText)){
        currentTimeBetween = 0;

        char nextChar = cachedText[textLength];
        displayText[textLength] = nextChar;
        textLength++;
    }

    Vector2 textSize = MeasureTextEx(GetFontDefault(), displayText, DefaultFontSize, spacing);

    if(maxTextWidth == 0 || textSize.x <= maxTextWidth){
        DrawTextEx(GetFontDefault(), displayText, textPosition, DefaultFontSize, spacing, color);
    }else{
        Vector2 drawLocation = textPosition;
        int wordCount = 0;
        const char** words = TextSplit(displayText, ' ', &wordCount);
        int wordOffset = 0;
        int currentWord = 0;

        for(int i = 0; i < textLength; i++){
            //printf("w:%d[i:%d, c:%c] o:%d | ", currentWord, i, displayText[i], wordOffset);
            DrawTextEx(GetFontDefault(), TextFormat("%c", displayText[i]), drawLocation, DefaultFontSize, spacing, color);
            drawLocation.x += MeasureTextEx(GetFontDefault(), TextFormat("%c", displayText[i]), DefaultFontSize, spacing).x;
            drawLocation.x += spacing;

            if(i >= TextLength(words[currentWord]) + wordOffset){
                wordOffset += TextLength(words[currentWord]) + 1;
                currentWord++;
            }
            if((displayText[i] == ' ' && wordCount > currentWord + 1 && MeasureTextEx(GetFontDefault(), words[currentWord], DefaultFontSize, spacing).x + drawLocation.x > maxTextWidth + textPosition.x) || drawLocation.x - textPosition.x > maxTextWidth){
                //printf("/skip/\n");
                drawLocation.y += spaceBetweenLines;
                drawLocation.x = textPosition.x;
            }
        }
        //printf("\n");
    }
}

void NewDisplayText(const char* text, Vector2 pos, int maxWidth){
    if(cachedText != NULL) free(cachedText);
    if(displayText != NULL) free(displayText);
    cachedText = (char*)calloc(TextLength(text), sizeof(char));
    displayText = (char*)calloc(TextLength(text), sizeof(char));
    TextCopy(cachedText, text);
    textPosition = pos;
    maxTextWidth = maxWidth;
    displayTextEnabled = true;
}

void ClearDisplayText(){
    free(displayText);
    free(cachedText);
    cachedText = NULL;
    displayText = NULL;
    displayTextEnabled = false;
}
