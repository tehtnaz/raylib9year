#include <stdio.h>
#include "raylib.h"
#include "animation.h"


// Animation.h version 3.0

Animation assignProperties(int spriteWidth, int currentFrame, int fps, bool isAnimating, int frameCount, bool allowsnap){
    Animation temp;

    temp.spriteWidth = spriteWidth;

    temp.frameCount = frameCount;
    temp.fps = fps;
    temp.currentFrame = currentFrame;
    temp.isAnimating = isAnimating;
    temp.allowSnap = allowsnap;
    temp.cycleBackward = false;

    temp.frameBuffer = 0;
    return temp;
}

// SwitchAnimation switchAssignProperties(int currentFrame, int fps, bool isAnimation){
//     SwitchAnimation temp;
//     temp.fps = fps;
//     temp.isAnimating = isAnimation;
//     temp.currentFrame = currentFrame;
//     return temp;
// }



Animation GetAnimationFromFolder(Animation input, bool autoSize, const char* path){
    Animation temp = input;
    char str[70];
    sprintf(str, "%s0.png", path);
    //const char* str = TextFormat("%s0.png", path);
    printf("GetAnimationFromFolder: Base Image = %s\n", str);
    Image img = LoadImage(str);

    if(autoSize){
        temp.spriteWidth = img.width;
        printf("GetAnimationFromFolder: animation spriteWidth (AUTO): %d\n", temp.spriteWidth);
    }else{
        printf("GetAnimationFromFolder: animation spriteWidth: %d\n", temp.spriteWidth);
    }
    Image atlas = GenImageColor(img.width * input.frameCount, img.height, BLANK);
    for(int i = 0; i < input.frameCount; i++){
        sprintf(str, "%s%d.png", path, i);
        img = LoadImage(str);
        //img = LoadImage(TextFormat("%s%d.png", path, i));
        ImageDraw(&atlas, img, (Rectangle){0,0,img.width,img.height}, (Rectangle){temp.spriteWidth * i,0,img.width,img.height}, WHITE);
    }
    UnloadImage(img);
    temp.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    return temp;
}

// auto sizes by default
Texture GetTextureAtlasFromFolder(const char* path, int textureCount){
    char str[70];
    sprintf(str, "%s0.png", path);
    //const char* str = TextFormat("%s0.png", path);
    printf("GetAnimationFromFolder: Base Image = %s\n", str);
    Image img = LoadImage(str);

    Image atlas = GenImageColor(img.width * textureCount, img.height, BLANK);
    for(int i = 0; i < textureCount; i++){
        sprintf(str, "%s%d.png", path, i);
        img = LoadImage(str);
        //img = LoadImage(TextFormat("%s%d.png", path, i));
        ImageDraw(&atlas, img, (Rectangle){0,0,img.width,img.height}, (Rectangle){img.width * i,0,img.width,img.height}, WHITE);
    }
    Texture tempTexture = LoadTextureFromImage(atlas);
    UnloadImage(img);
    UnloadImage(atlas);
    return tempTexture;
}

// SwitchAnimation GetSwitchAnimationFromFolder(SwitchAnimation input, const char* path){
//     SwitchAnimation temp = input;
//     temp.frames[0] = LoadTexture(TextFormat("%s0.png", path));
//     temp.frames[1] = LoadTexture(TextFormat("%s1.png", path));
//     return temp;
// }



void CycleAnimation(Animation* input){
    if(input->isAnimating == false){
        printf("Warning: CycleAnimation - Attempted to cycle an animation which was disabled.\n");
        return;
    }
    if(input->fps < 0){
        printf("ERROR: CycleAnimation - Attempted to cycle with negative fps\n");
        return;
    }

    input->frameBuffer += input->fps * GetFrameTime();
    while(input->frameBuffer >= 1){
            input->frameBuffer--;
            input->currentFrame++;
    }
    if(input->currentFrame > input->frameCount - 1 && input->allowSnap){
        input->currentFrame = 0;
    }
    if(input->currentFrame >= input->frameCount - 1 && !input->allowSnap){
        input->currentFrame = input->frameCount - 1;
        input->isAnimating = false;
    }
}

void CycleAnimationBackwards(Animation* input){
    if(input->isAnimating == false){
        printf("Warning: CycleAnimationBackwards - Attempted to cycle an animation which was disabled.\n");
        return;
    }
    if(input->fps < 0){
        printf("ERROR: CycleAnimationBackwards - Attempted to cycleBackwards with negative fps\n");
        return;
    }

    input->frameBuffer += input->fps * GetFrameTime();

    while(input->frameBuffer >= 1){
            input->frameBuffer--;
            input->currentFrame--;
    }

    if(input->currentFrame < 0 && input->allowSnap){
        input->currentFrame = input->frameCount - 1;
    }
    if(input->currentFrame <= 0 && !input->allowSnap){
        input->currentFrame = 0;
        input->isAnimating = false;
    }
}

//ignores isAnimating property and only works while allowSnap property is disabled

void ShakeCycleAnimation(Animation* input){
    if(input->allowSnap){
        printf("allowSnap is enabled for this object. Did you mean to cycle it and not shake it? Skipping...\n");
        return;
    }
    if(input->isAnimating){
        if(!input->cycleBackward){
            //printf("cycle\n");
            return CycleAnimation(input);
        }else{
            //printf("cycleBack\n");
            return CycleAnimationBackwards(input);
        }
    }else{
        input->cycleBackward = !input->cycleBackward;
        input->isAnimating = true;
        if(!input->cycleBackward){
            //printf("cycle+switch\n");
            return CycleAnimation(input);
        }else{
            //printf("cycleBack+switch\n");
            return CycleAnimationBackwards(input);
        }
    }
}

void DrawAnimationPro(Animation* input, Vector2 position, float scale, Color tint, CycleType cycleAnim){
    DrawTexturePro(input->texture,
                    (Rectangle){input->currentFrame * input->spriteWidth, 0, input->spriteWidth, input->texture.height},
                    (Rectangle){0,0,scale*input->spriteWidth,scale*input->texture.height},
                    (Vector2){-position.x, -position.y},0,tint);
    switch(cycleAnim){
        case CYCLE_NONE:
            break;
        case CYCLE_FORWARD:
            CycleAnimation(input);
            break;
        case CYCLE_BACKWARD:
            CycleAnimationBackwards(input);
            break;
        case CYCLE_SHAKE:
            ShakeCycleAnimation(input);
            break;
    }
}

void DrawTextureFromAtlas(Texture2D atlas, int spriteID, int spriteCountInAtlas, Vector2 position, float scale, Color tint){
    const int spriteWidth = atlas.width / spriteCountInAtlas;
    DrawTexturePro(atlas,
                    (Rectangle){spriteID * spriteWidth, 0, spriteWidth, atlas.height},
                    (Rectangle){0,0,scale*spriteWidth,scale*atlas.height},
                    (Vector2){-position.x, -position.y},0,tint);
}


Animation FlipAnimationHorizontal(Animation input){
    Animation temp = input;
    Image animTexture = LoadImageFromTexture(temp.texture);
    printf("a, %d %d; %d %d", animTexture.width, animTexture.height, temp.texture.width, temp.texture.height);
    Image tempImg = GenImageColor(temp.spriteWidth, temp.texture.height, BLANK);
    printf("b");
    for(int i = 0; i < temp.frameCount; i++){
        tempImg = GenImageColor(temp.spriteWidth, temp.texture.height, BLANK);
        ImageDraw(&tempImg, animTexture, (Rectangle){i * temp.spriteWidth, 0, temp.spriteWidth, temp.texture.height}, (Rectangle){0, 0,temp.spriteWidth, temp.texture.height}, WHITE);
        ImageDrawRectangle(&animTexture, i * temp.spriteWidth, 0, temp.spriteWidth, temp.texture.height, BLANK);
        ImageFlipHorizontal(&tempImg);
        ImageDraw(&animTexture, tempImg, (Rectangle){0, 0,temp.spriteWidth, temp.texture.height}, (Rectangle){i * temp.spriteWidth, 0, temp.spriteWidth, temp.texture.height}, WHITE);
        printf("d");
    }
    printf("c");
    UnloadTexture(temp.texture);
    temp.texture = LoadTextureFromImage(animTexture);
    UnloadImage(tempImg);
    UnloadImage(animTexture);
    return temp;
}

// SwitchAnimation FlipSwitchAnimationHorizontal(SwitchAnimation input){
//     SwitchAnimation temp = input;
//     Image tempImg0 = LoadImageFromTexture(temp.frames[0]);
//     Image tempImg1 = LoadImageFromTexture(temp.frames[1]);
//     UnloadTexture(temp.frames[0]);
//     UnloadTexture(temp.frames[1]);

//     ImageFlipHorizontal(&tempImg0);
//     ImageFlipHorizontal(&tempImg1);

//     temp.frames[0] = LoadTextureFromImage(tempImg0);
//     temp.frames[1] = LoadTextureFromImage(tempImg1);
//     UnloadImage(tempImg0);
//     UnloadImage(tempImg1);
//     return temp;
// }