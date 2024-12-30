#include "displayText.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

// Constants
#define MinTimeBetween 0.03f
#define DefaultFontSize 10.0f
#define DefaultLineSpacing 1.15f;
#define MAX_QUEUED_TEXT 16
static const float spaceBetweenLines = DefaultFontSize * DefaultLineSpacing;
static const float spacing = DefaultFontSize / 10;
static Font defaultFont = {0};

// Static Variables
static char* cachedText = NULL;
static char* displayText = NULL;

static int selectedText = 0;
static int queuedTextCount = 0;
static char* queuedText[MAX_QUEUED_TEXT];
static float queuedMaxTextWidth[MAX_QUEUED_TEXT];
static Vector2 queuedTextPosition[MAX_QUEUED_TEXT];


static float maxTextWidth = 0;
static bool displayTextEnabled = false;
static Vector2 textPosition = {0, 0};
static float currentTimeBetween = 0;

void InitDisplayText(){
    cachedText = NULL;
    displayText = NULL;
    for(int i = 0; i < MAX_QUEUED_TEXT; i++){
        queuedText[i] = NULL;
        queuedMaxTextWidth[i] = 0;
        queuedTextPosition[i] = (Vector2){0, 0};
    }
    // SetTextLineSpacing(spaceBetweenLines);
    // This may seem useless, bc raylib uses the default font without the *Ex() functions, 
    // but maybe we could change the font someday? I think that's what I intended originally
    defaultFont = GetFontDefault();
}

// void EndDisplayText(){}

// Allocate new text to be shown across the screen
void NewDisplayText(const char* text, Vector2 pos, int maxWidth){
    if(cachedText != NULL) free(cachedText);
    if(displayText != NULL) free(displayText);
    cachedText = calloc(TextLength(text), sizeof(char));
    displayText = calloc(TextLength(text), sizeof(char));
    TextCopy(cachedText, text);
    textPosition = pos;
    maxTextWidth = maxWidth;
    displayTextEnabled = true;
    // if(defaultFont.baseSize == 0){
    //     //defaultFont = LoadFont("./../res/Text/DotGothic16-Regular.ttf");
    //     //SetTextureFilter(defaultFont.texture, TEXTURE_FILTER_POINT);
    //     defaultFont = GetFontDefault();
    // }
}

void QueueDisplayText(const char* item, Vector2 pos, int maxWidth){
    if(queuedTextCount == MAX_QUEUED_TEXT){
        TraceLog(LOG_ERROR, "displayText -  Could not create queue more texts (max reached)");
        return;
    }
    int length = TextLength(item);
    queuedText[queuedTextCount] = calloc(length, sizeof(char));
    TextCopy(queuedText[queuedTextCount], item);

    queuedTextPosition[queuedTextCount] = pos;
    queuedMaxTextWidth[queuedTextCount] = maxWidth;

    queuedTextCount++;

    #ifdef _DEBUG
        TraceLog(LOG_DEBUG, "displayText - Item queued. Length: %d, Text:[[%s]]", length, item);
    #endif
}

// Clear existing text
void ClearDisplayText(){
    if(displayText != NULL) free(displayText);
    else TraceLog(LOG_WARNING, "Tried freeing NULL displayText");
    if(cachedText != NULL) free(cachedText);
    else TraceLog(LOG_WARNING, "Tried freeing NULL cachedText");
    
    cachedText = NULL;
    displayText = NULL;
    displayTextEnabled = false;
}

void ClearDisplayTextQueue(){
    for(int i = 0; i < queuedTextCount; i++){
        if(queuedText[i] != NULL) free(queuedText[i]);
        else TraceLog(LOG_WARNING, "Tried freeing NULL queuedText");
        queuedText[i] = NULL;
    }
    queuedTextCount = 0;
    selectedText = 0;
}

bool GetDisplayTextEnabled(){
    return displayTextEnabled;
}

// MUST BE RUN ON EVERY FRAME
// Word wraps text with current width

// ! redo all this shit down here

void UpdateAndDrawTypingText(Color color){
    if(!displayTextEnabled){
        
        if(selectedText < queuedTextCount){
            TraceLog(LOG_DEBUG, "UpdateAndDrawTypingText - shifting...");
            TraceLog(LOG_DEBUG, "UpdateAndDrawTypingText - %s", queuedText[selectedText]);
            NewDisplayText(queuedText[selectedText], queuedTextPosition[selectedText], queuedMaxTextWidth[selectedText]);
            selectedText++;
            // for(int i = 0; i < queuedTextCount; i++){
            //     queuedText[i] = queuedText[i+1];
            //     queuedTextPosition[i] = queuedTextPosition[i+1];
            //     queuedMaxTextWidth[i] = queuedMaxTextWidth[i+1];
            // }
            for(int i = 0; i < queuedTextCount; i++){
                TraceLog(LOG_DEBUG, "bruh fr??: [[%p]] ((%s))", queuedText[i], queuedText[i]);
            }
        }else{
            return;
        }
    }

    int textLength = TextLength(displayText);

    currentTimeBetween += GetFrameTime();
    if(currentTimeBetween >= MinTimeBetween && textLength != TextLength(cachedText)){
        currentTimeBetween = 0;

        char nextChar = cachedText[textLength];
        displayText[textLength] = nextChar;
        textLength++;
    }

    Vector2 textSize = MeasureTextEx(defaultFont, displayText, DefaultFontSize, spacing);
    // int textSize = MeasureText(displayText, DefaultFontSize);

    if(maxTextWidth == 0 || textSize.x <= maxTextWidth){
    // if(maxTextWidth == 0 || textSize <= maxTextWidth){
        DrawTextEx(defaultFont, displayText, textPosition, DefaultFontSize, spacing, color);
        // DrawText(displayText, textPosition.x, textPosition.y, DefaultFontSize, color);
    }else{
        Vector2 drawLocation = textPosition;
        int wordCount = 0;
        const char** words = TextSplit(displayText, ' ', &wordCount);
        int wordOffset = 0;
        int currentWord = 0;

        for(int i = 0; i < textLength; i++){
            //printf("w:%d[i:%d, c:%c] o:%d | ", currentWord, i, displayText[i], wordOffset);
            DrawTextEx(defaultFont, TextFormat("%c", displayText[i]), drawLocation, DefaultFontSize, spacing, color);
            // DrawText(TextFormat("%c", displayText[i]), drawLocation.x, drawLocation.y, DefaultFontSize, color);
            drawLocation.x += MeasureTextEx(defaultFont, TextFormat("%c", displayText[i]), DefaultFontSize, spacing).x;
            // drawLocation.x += MeasureText(TextFormat("%c", displayText[i]), DefaultFontSize);
            drawLocation.x += spacing;

            if(i >= TextLength(words[currentWord]) + wordOffset){
                wordOffset += TextLength(words[currentWord]) + 1;
                currentWord++;
            }
            if((displayText[i] == ' ' && wordCount > currentWord + 1 && MeasureTextEx(defaultFont, words[currentWord], DefaultFontSize, spacing).x + drawLocation.x > maxTextWidth + textPosition.x) || drawLocation.x - textPosition.x > maxTextWidth){
            // if((displayText[i] == ' ' && wordCount > currentWord + 1 && MeasureText(words[currentWord], DefaultFontSize) + drawLocation.x > maxTextWidth + textPosition.x) || drawLocation.x - textPosition.x > maxTextWidth){
                drawLocation.y += spaceBetweenLines;
                drawLocation.x = textPosition.x;
            }
        }
    }
}