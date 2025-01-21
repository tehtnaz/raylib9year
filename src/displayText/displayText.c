#include "displayText.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Constants
#define MinTimeBetween 0.03f
#define DefaultFontSize 10.0f
#define DefaultLineSpacing 1 // (in pixels)
#define DefaultCharacterSpacing DefaultFontSize / 10 // divided by ten b/c that's the normal size of the text glyphs
#define MAX_QUEUED_TEXT 16
#define MAX_STRING_SIZE 256
static Font defaultFont = {0};

// Static Variables
static char cachedText[MAX_STRING_SIZE * 2] = {0}; // cached text including \r; has enough space for \r after each character
static char displayText[MAX_STRING_SIZE] = {0}; // text actually displayed on screen

static int selectedText = 0;
static int queuedTextCount = 0;
static char* queuedText[MAX_QUEUED_TEXT];
static float queuedMaxTextWidth[MAX_QUEUED_TEXT];
static Vector2 queuedTextPosition[MAX_QUEUED_TEXT];


//static float maxTextWidth = 0;
static bool displayTextEnabled = false;
static Vector2 textPosition = {0, 0};
static float currentTimeBetween = 0;

// Inserts carriage return at next index after indexOfLastChar. indexOfLastChar cannot be negative
void InsertCarriageReturn(int indexOfLastChar, char* str, int arraySize){
    // one for \0 and one for the new carriage return \r
    if(str == NULL) return;
    if(TextLength(str) + 2 > arraySize){
        TraceLog(LOG_WARNING, "displayText - Couldn't insert new carriage return as it exceeded the arraySize (Your string is either too long or causes too many line breaks when wrapping)");
        return;
    }
    str[TextLength(str) + 1] = '\0';
    for(int i = TextLength(str); i > indexOfLastChar + 1; i--){
        str[i] = str[i - 1];
    }
    str[indexOfLastChar + 1] = '\r';
}
void RemoveCarriageReturn(int indexOfCarriageReturn, char* str){
    // one for \0 and one for the new carriage return \r
    if(str == NULL) return;
    int len = TextLength(str);
    for(int i = indexOfCarriageReturn; i < len; i++){
        str[i] = str[i + 1];
    }
}


void InitDisplayText(){
    memset(cachedText, 0, MAX_STRING_SIZE * 2);
    memset(displayText, 0, MAX_STRING_SIZE);
    
    for(int i = 0; i < MAX_QUEUED_TEXT; i++){
        queuedText[i] = NULL;
        queuedMaxTextWidth[i] = 0;
        queuedTextPosition[i] = (Vector2){0, 0};
    }
    SetTextLineSpacing(DefaultLineSpacing);
    // This may seem useless, bc raylib uses the default font without the *Ex() functions, 
    // but maybe we could change the font someday? I think that's what I intended originally
    defaultFont = GetFontDefault();
}

// void EndDisplayText(){}

// Allocate new text to be shown across the screen
// Carriage returns (\r) are inserted when a line hits its max size
void NewDisplayText(const char* text, Vector2 pos, int maxWidth){
    if(TextLength(text) + 1 > MAX_STRING_SIZE){
        TraceLog(LOG_WARNING, "displayText - Couldn't load new display text as it exceeded the MAX_STRING_SIZE");
        return;
    }
    TextCopy(cachedText, text);
    int textLength = TextLength(text);
    int maxLength = 0;
    int carriageReturnCount = 0;
    int lastValidPoint = 0;
    for(int i = 0; i < textLength; i++){
        // idk what happens if the word is over the space limit
        Vector2 size = MeasureTextEx(defaultFont, TextSubtext(text, lastValidPoint, i + 1 - lastValidPoint), DefaultFontSize, DefaultCharacterSpacing);
        if(size.x > maxWidth){
            int j = i;
            while (j != 0 && text[j] != ' '){
                j--;
            }
            InsertCarriageReturn(j + 1 + carriageReturnCount, cachedText, MAX_STRING_SIZE * 2);
            lastValidPoint = j + 1;
        }
        //cachedText[i + carriageReturnCount] = text[i];
    }
    textPosition = pos;
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
    queuedText[queuedTextCount] = calloc(length + 1, sizeof(char));
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
    memset(cachedText, 0, MAX_STRING_SIZE * 2);
    memset(displayText, 0, MAX_STRING_SIZE);
    // if(displayText != NULL) free(displayText);
    // else TraceLog(LOG_WARNING, "Tried freeing NULL displayText");
    // if(cachedText != NULL) free(cachedText);
    // else TraceLog(LOG_WARNING, "Tried freeing NULL cachedText");
    
    // cachedText = NULL;
    // displayText = NULL;
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
// Carriage returns (\r) are used to indicate where a line hits its max size
// ! redo all this shit down here

void UpdateAndDrawTypingText(Color color){
    if(!displayTextEnabled){
        
        if(selectedText < queuedTextCount){
            TraceLog(LOG_DEBUG, "UpdateAndDrawTypingText - shifting...");
            TraceLog(LOG_DEBUG, "UpdateAndDrawTypingText - %s", queuedText[selectedText]);
            NewDisplayText(queuedText[selectedText], queuedTextPosition[selectedText], queuedMaxTextWidth[selectedText]);
            selectedText++;

            for(int i = 0; i < queuedTextCount; i++){
                TraceLog(LOG_DEBUG, "bruh fr??: [[%p]] ((%s))", queuedText[i], queuedText[i]);
            }
        }else{
            return;
        }
    }

    int displayTextLength = TextLength(displayText);

    currentTimeBetween += GetFrameTime();
    if(currentTimeBetween >= MinTimeBetween && displayTextLength != TextLength(cachedText)){
        currentTimeBetween = 0;

        char nextChar = cachedText[displayTextLength];
        if(nextChar == '\r'){
            RemoveCarriageReturn(displayTextLength, cachedText);
            nextChar = cachedText[displayTextLength];
            int j = displayTextLength;
            while (j != 0 && displayText[j] != ' '){
                j--;
            }
            displayText[j] = '\n';
        }
        displayText[displayTextLength] = nextChar;
        displayTextLength++;
    }

    DrawTextEx(defaultFont, displayText, textPosition, DefaultFontSize, DefaultCharacterSpacing, color);

    // Vector2 textSize = MeasureTextEx(defaultFont, displayText, DefaultFontSize, spacing);
    // // int textSize = MeasureText(displayText, DefaultFontSize);

    // if(maxTextWidth == 0 || textSize.x <= maxTextWidth){
    // // if(maxTextWidth == 0 || textSize <= maxTextWidth){
    //     DrawTextEx(defaultFont, displayText, textPosition, DefaultFontSize, spacing, color);
    //     // DrawText(displayText, textPosition.x, textPosition.y, DefaultFontSize, color);
    // }else{
    //     Vector2 drawLocation = textPosition;
    //     int wordCount = 0;
    //     const char** words = TextSplit(displayText, ' ', &wordCount);
    //     int wordOffset = 0;
    //     int currentWord = 0;

    //     for(int i = 0; i < displayTextLength; i++){
    //         //printf("w:%d[i:%d, c:%c] o:%d | ", currentWord, i, displayText[i], wordOffset);
    //         DrawTextEx(defaultFont, TextFormat("%c", displayText[i]), drawLocation, DefaultFontSize, spacing, color);
    //         // DrawText(TextFormat("%c", displayText[i]), drawLocation.x, drawLocation.y, DefaultFontSize, color);
    //         drawLocation.x += MeasureTextEx(defaultFont, TextFormat("%c", displayText[i]), DefaultFontSize, spacing).x;
    //         // drawLocation.x += MeasureText(TextFormat("%c", displayText[i]), DefaultFontSize);
    //         drawLocation.x += spacing;

    //         if(i >= TextLength(words[currentWord]) + wordOffset){
    //             wordOffset += TextLength(words[currentWord]) + 1;
    //             currentWord++;
    //         }
    //         if((displayText[i] == ' ' && wordCount > currentWord + 1 && MeasureTextEx(defaultFont, words[currentWord], DefaultFontSize, spacing).x + drawLocation.x > maxTextWidth + textPosition.x) || drawLocation.x - textPosition.x > maxTextWidth){
    //         // if((displayText[i] == ' ' && wordCount > currentWord + 1 && MeasureText(words[currentWord], DefaultFontSize) + drawLocation.x > maxTextWidth + textPosition.x) || drawLocation.x - textPosition.x > maxTextWidth){
    //             drawLocation.y += spaceBetweenLines;
    //             drawLocation.x = textPosition.x;
    //         }
    //     }
    // }
}