#ifndef ANIMATION_H
    #define ANIMATION_H
    #include "raylib.h"

// TODO:
// Maybe split all the sections of the main.c loop into different functions
//try to fix prepare level
//with all source files containing texutre loading, add texture unloading maybe? (ie. UnloadAnimation)



//Animation.h version 3.0

// Changes from version 1 (present in sandwich factory)
//  | allowSnap has new definition (defined on line 30)
//  | warns you when attempting to cycle an animation that is not ".isAnimating"

// 1.2 changelog
//  | added shakeCycleAnimation

// 2.0 changelog
//  | added drawAnimation
//  | BIG: changed frame array to texture atlas
//  | shakeCycleAnimation now relies on negative FPS. Try not to use negative FPS while doing regular anims
//  | WATCH OUT: cycleAnimation and cycleAnimationBackwards will still not accept negative FPS!

// 2.1 changelog
//  | use GetFPS instead of screenFPS
//  | fixed flipAnimationHorizontal
//  | fixed error printf (added new line char)
// 2.2 changelog
//  | removed vector2pp.h dependancy

// 3.0 changelog
//  | removed SwitchAnimation (temporary?)
//  | added DrawTextureFromAtlas
//  | spriteSize renamed to spriteWidth (might be removed, see comment)
//  | some functions renamed to use TitleCase, or to be more specifc
//  | cycling funcs no longer return animation, pass in pointer instead
//  | changed DrawTextureTiled calls to DrawTexturePro
//  | reformatted this changelog

// 3.1 changelog
//  | fix debug info

// 3.2 changelog
//  | use TraceLog for logging

typedef struct Animation{
    Texture2D texture; // sprite sheet
    int spriteWidth; //width, since atlas height is defualt height of sprite which also means you can't stack vertically
    // ! Could be removed (texture.width / frameCount = spriteWidth)
    
    int frameCount; // Defines where we stop reading data  (eg. 0 to 5  = 6 items)
    int fps; //Instead of the rendered fps, use whatever you like! Only accepts negative fps if doing shakeCycle (still not recommended)
    int currentFrame;
    
    bool isAnimating;
    bool allowSnap; // This defines whether the currentFrame will reset to 0 once it overflows, or whether it won't. Vice versa for underflowing. If false, it will stop the animation.
    bool cycleBackward;
    
    float frameBuffer; //Amount of frames since last switch. This is scaled to the rendered FPS so that this animation can have any speed you want
}Animation;

// not using a sprite sheet makes multi-size textures better
// typedef struct SwitchAnimation{
//     Texture2D frames[2];  // use texture pointer for resizeable array in future
//     int fps;
//     bool isAnimating;
//     int currentFrame;
// }SwitchAnimation;

typedef enum CycleType{
    CYCLE_NONE,
    CYCLE_FORWARD,
    CYCLE_BACKWARD,
    CYCLE_SHAKE
}CycleType;

Animation assignProperties(int spriteWidth, int currentFrame, int fps, bool isAnimating, int frameCount, bool allowsnap);
// SwitchAnimation switchAssignProperties(int currentFrame, int fps, bool isAnimation);

Animation GetAnimationFromFolder(Animation input, bool autoSize, const char* path);
// SwitchAnimation GetSwitchAnimationFromFolder(SwitchAnimation input, const char* path);

Texture GetTextureAtlasFromFolder(const char* path, int textureCount);

void CycleAnimation(Animation* input);
void CycleAnimationBackwards(Animation* input);
void ShakeCycleAnimation(Animation* input); //ignores isAnimating property and only works while allowSnap property is disabled

void DrawAnimationPro(Animation* input, Vector2 position, float scale, Color tint, CycleType cycleAnim);
void DrawTextureFromAtlas(Texture2D atlas, int spriteID, int spriteCountInAtlas, Vector2 position, float scale, Color tint);

Animation FlipAnimationHorizontal(Animation input);
// SwitchAnimation FlipSwitchAnimationHorizontal(SwitchAnimation input);

#endif //animation_h version 3.2