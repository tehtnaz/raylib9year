#define PHYSAC_IMPLEMENTATION
#include "physac.h"
// Shatters a polygon shape physics body to little physics bodies with explosion force
// TODO: not modified with new props
void PhysicsShatter(PhysicsBody body, Vector2 position, float force)
{
    if (body != NULL)
    {
        if (body->shape.type == PHYSICS_POLYGON)
        {
            PhysicsVertexData vertexData = body->shape.vertexData;
            bool collision = false;

            for (unsigned int i = 0; i < vertexData.vertexCount; i++)
            {
                Vector2 positionA = body->position;
                Vector2 positionB = MathMatVector2Product(body->shape.transform, MathVector2Add(body->position, vertexData.positions[i]));
                unsigned int nextIndex = (((i + 1) < vertexData.vertexCount) ? (i + 1) : 0);
                Vector2 positionC = MathMatVector2Product(body->shape.transform, MathVector2Add(body->position, vertexData.positions[nextIndex]));

                // Check collision between each triangle
                float alpha = ((positionB.y - positionC.y)*(position.x - positionC.x) + (positionC.x - positionB.x)*(position.y - positionC.y))/
                              ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float beta = ((positionC.y - positionA.y)*(position.x - positionC.x) + (positionA.x - positionC.x)*(position.y - positionC.y))/
                             ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float gamma = 1.0f - alpha - beta;

                if ((alpha > 0.0f) && (beta > 0.0f) & (gamma > 0.0f))
                {
                    collision = true;
                    break;
                }
            }

            if (collision)
            {
                int count = vertexData.vertexCount;
                Vector2 bodyPos = body->position;
                Vector2 *vertices = (Vector2 *)PHYSAC_MALLOC(sizeof(Vector2)*count);
                Matrix2x2 trans = body->shape.transform;
                for (int i = 0; i < count; i++) vertices[i] = vertexData.positions[i];

                // Destroy shattered physics body
                DestroyPhysicsBody(body);

                for (int i = 0; i < count; i++)
                {
                    int nextIndex = (((i + 1) < count) ? (i + 1) : 0);
                    Vector2 center = MathTriangleBarycenter(vertices[i], vertices[nextIndex], PHYSAC_VECTOR_ZERO);
                    center = MathVector2Add(bodyPos, center);
                    Vector2 offset = MathVector2Subtract(center, bodyPos);

                    PhysicsBody body = CreatePhysicsBodyPolygon(center, 10, 3, 10, body->trigger);     // Create polygon physics body with relevant values

                    PhysicsVertexData vertexData = { 0 };
                    vertexData.vertexCount = 3;

                    vertexData.positions[0] = MathVector2Subtract(vertices[i], offset);
                    vertexData.positions[1] = MathVector2Subtract(vertices[nextIndex], offset);
                    vertexData.positions[2] = MathVector2Subtract(position, center);

                    // Separate vertices to avoid unnecessary physics collisions
                    vertexData.positions[0].x *= 0.95f;
                    vertexData.positions[0].y *= 0.95f;
                    vertexData.positions[1].x *= 0.95f;
                    vertexData.positions[1].y *= 0.95f;
                    vertexData.positions[2].x *= 0.95f;
                    vertexData.positions[2].y *= 0.95f;

                    // Calculate polygon faces normals
                    for (unsigned int j = 0; j < vertexData.vertexCount; j++)
                    {
                        unsigned int nextVertex = (((j + 1) < vertexData.vertexCount) ? (j + 1) : 0);
                        Vector2 face = MathVector2Subtract(vertexData.positions[nextVertex], vertexData.positions[j]);

                        vertexData.normals[j] = CLITERAL(Vector2){ face.y, -face.x };
                        MathVector2Normalize(&vertexData.normals[j]);
                    }

                    // Apply computed vertex data to new physics body shape
                    body->shape.vertexData = vertexData;
                    body->shape.transform = trans;

                    // Calculate centroid and moment of inertia
                    center = PHYSAC_VECTOR_ZERO;
                    float area = 0.0f;
                    float inertia = 0.0f;

                    for (unsigned int j = 0; j < body->shape.vertexData.vertexCount; j++)
                    {
                        // Triangle vertices, third vertex implied as (0, 0)
                        Vector2 p1 = body->shape.vertexData.positions[j];
                        unsigned int nextVertex = (((j + 1) < body->shape.vertexData.vertexCount) ? (j + 1) : 0);
                        Vector2 p2 = body->shape.vertexData.positions[nextVertex];

                        float D = MathVector2CrossProduct(p1, p2);
                        float triangleArea = D/2;

                        area += triangleArea;

                        // Use area to weight the centroid average, not just vertex position
                        center.x += triangleArea*PHYSAC_K*(p1.x + p2.x);
                        center.y += triangleArea*PHYSAC_K*(p1.y + p2.y);

                        float intx2 = p1.x*p1.x + p2.x*p1.x + p2.x*p2.x;
                        float inty2 = p1.y*p1.y + p2.y*p1.y + p2.y*p2.y;
                        inertia += (0.25f*PHYSAC_K*D)*(intx2 + inty2);
                    }

                    center.x *= 1.0f/area;
                    center.y *= 1.0f/area;

                    body->mass = area;
                    body->inverseMass = ((body->mass != 0.0f) ? 1.0f/body->mass : 0.0f);
                    body->inertia = inertia;
                    body->inverseInertia = ((body->inertia != 0.0f) ? 1.0f/body->inertia : 0.0f);

                    // Calculate explosion force direction
                    Vector2 pointA = body->position;
                    Vector2 pointB = MathVector2Subtract(vertexData.positions[1], vertexData.positions[0]);
                    pointB.x /= 2.0f;
                    pointB.y /= 2.0f;
                    Vector2 forceDirection = MathVector2Subtract(MathVector2Add(pointA, MathVector2Add(vertexData.positions[0], pointB)), body->position);
                    MathVector2Normalize(&forceDirection);
                    forceDirection.x *= force;
                    forceDirection.y *= force;

                    // Apply force to new physics body
                    PhysicsAddForce(body, forceDirection);
                }

                PHYSAC_FREE(vertices);
            }
        }
    }
    else TRACELOG("[PHYSAC] WARNING: PhysicsShatter: NULL physic body\n");
}

// Returns the barycenter of a triangle given by 3 points
static Vector2 MathTriangleBarycenter(Vector2 v1, Vector2 v2, Vector2 v3)
{
    Vector2 result = { 0.0f, 0.0f };

    result.x = (v1.x + v2.x + v3.x)/3;
    result.y = (v1.y + v2.y + v3.y)/3;

    return result;
}