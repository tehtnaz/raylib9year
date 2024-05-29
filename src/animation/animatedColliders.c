#include "animatedColliders.h"
#include <stdlib.h>

static int animatedColliderCount = 0;
static AnimatedCollider animatedColliderArray[MAX_ANIMATED_COLLIDERS];


// ColliderShift RectangleToCollider(Rectangle rect){
//     return (ColliderShift){(Vector2){rect.x + rect.width / 2, rect.y + rect.height / 2}, (Vector2){rect.width, rect.height}};
// }

void UpdateAnimCollider(AnimatedCollider anim){
    // if(anim.body->position.y != anim.startPosition.y && anim.body->position.y != anim.endPosition.y){
    //     TraceLog(LOG_DEBUG, "Animating Collider - startPos: (x: %f, y: %f) currentPos: (x: %f, y: %f) frameNum: %d", anim.startPosition.x, anim.startPosition.y, anim.body->position.x, anim.body->position.y, anim.linkedAnim->currentFrame);
    // }
    anim.body->position = Vector2Lerp(anim.startPosition, anim.endPosition, (float)anim.linkedAnim->currentFrame / (float)anim.linkedAnim->frameCount);
    // body->shape.vertexData
}

AnimatedCollider* CreateAnimatedCollider(Vector2 startPosition, Vector2 endPosition, PhysicsBody body, Animation* linkedAnim){
    if(animatedColliderCount == MAX_ANIMATED_COLLIDERS){
        TraceLog(LOG_WARNING, "animatedColliders - animatedCollider limit reached. Couldn't create new animatedCollider");
        return NULL;
    }
    animatedColliderArray[animatedColliderCount] = (AnimatedCollider){body, startPosition, endPosition, linkedAnim};
    animatedColliderCount++;

    return &(animatedColliderArray[animatedColliderCount - 1]);
}

void UpdateAnimatedColliders(){
    for(int i = 0; i < animatedColliderCount; i++){
        UpdateAnimCollider(animatedColliderArray[i]);
    }
}

Animation* FindAnimationFromPhysicsBody(PhysicsBody body){
    TraceLog(LOG_DEBUG, "Finding AnimatedCol from Body %d", body->trigger);
    for(int i = 0; i < animatedColliderCount; i++){
        if(animatedColliderArray[i].body == body){
            return animatedColliderArray[i].linkedAnim;
        }
    }
    return NULL;
}

void DestroyAllAnimatedColliders(){
    animatedColliderCount = 0;
}