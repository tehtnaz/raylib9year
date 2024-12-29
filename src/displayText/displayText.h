#pragma once

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

// Should only be run once!
void InitDisplayText(); // initialize
// void EndDisplayText();  // tie loose ends (clear everything)

// MUST BE RUN ON EVERY FRAME
// Word wraps text with current width
void UpdateAndDrawTypingText(Color color);

// Queue new text to be shown across the screen
void QueueDisplayText(const char* item, Vector2 pos, int maxWidth);

// Clear existing text
void ClearDisplayTextQueue();
void ClearDisplayText();

bool GetDisplayTextEnabled();
