#pragma once

#include "raylib.h"
#include "raymath.h"
#include "animation.h"
#include "physac.h"

#define MAX_ANIMATED_COLLIDERS 8

// typedef struct AnimatedCollider{
//     Collider* colliderKeyframes;
//     float* keyframeLengths;
//     float animProgression;
//     float rotation;
// }AnimatedCollider;

typedef struct AnimatedCollider{
    PhysicsBody body;
    ColliderShift initialCollider;
    ColliderShift finalCollider;
    Animation linkedAnim; // link a collider to a sprite anim
    // float keyframeLength; // in seconds
    // float animProgression; // how much the animation has progressed in percentage (1.00 == 100%, 0.50 == 50%, 0.00 = 0%) 
}AnimatedCollider;

ColliderShift RectangleToCollider(Rectangle rect);
AnimatedCollider* CreateAnimatedCollider(ColliderShift initial, ColliderShift final, PhysicsBody body, Animation linkedAnim);
void UpdateAnimatedColliders();