#include "animatedColliders.h"
#include <stdlib.h>

static int animatedColliderCount = 0;
static AnimatedCollider animatedColliderArray[MAX_ANIMATED_COLLIDERS];


// ColliderShift RectangleToCollider(Rectangle rect){
//     return (ColliderShift){(Vector2){rect.x + rect.width / 2, rect.y + rect.height / 2}, (Vector2){rect.width, rect.height}};
// }

void UpdateAnimCollider(AnimatedCollider anim){
    anim.body->position = Vector2Lerp(anim.initialCollider.offset, anim.finalCollider.offset, anim.linkedAnim.currentFrame / anim.linkedAnim.frameCount);
    // body->shape.vertexData
}

AnimatedCollider* CreateAnimatedCollider(ColliderShift initial, ColliderShift final, PhysicsBody body, Animation linkedAnim){
    if(animatedColliderCount == MAX_ANIMATED_COLLIDERS){
        TraceLog(LOG_WARNING, "animatedColliders - animatedCollider limit reached. Couldn't create new animatedCollider");
        return NULL;
    }
    animatedColliderArray[animatedColliderCount] = (AnimatedCollider){body, initial, final, linkedAnim};
    animatedColliderCount++;

    return &(animatedColliderArray[animatedColliderCount - 1]);
}

void UpdateAnimatedColliders(){
    for(int i = 0; i < animatedColliderCount; i++){
        UpdateAnimCollider(animatedColliderArray[i]);
    }
}