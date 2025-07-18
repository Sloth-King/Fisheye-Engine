#pragma once

#include <set>
#include <glm/vec3.hpp>
#include "engine/physics/components/components.h"
#include "engine/core/gameObject/gameObject.h"
#include "engine/physics/collider/collider.h"

struct intersectionData
{
    bool isIntersection;
    glm::vec3 intersectionPointA;
    glm::vec3 intersectionPointB;
    glm::vec3 intersectionNormal;
    GameObjectData *objectA;
    GameObjectData *objectB;
    Collider *colliderA;
    Collider *colliderB;
    float _overlapDistance; // _ for meta data
};

// Largely inspired by this
// https://winter.dev/articles/physics-engine

class PhysicsServer
{
private:
    std::set<GameObjectData *> Objects;
    glm::vec3 gravity = glm::vec3(0.0, -5, 0.0);
    float gameSpeed = 0.1f;
    float deltaT = 0.0f;

public:
    void addObject(GameObjectData *go);                // Add an object to the phys world
    void removeObject(GameObjectData *go);             // Remove an object from the phys world
    float getDeltaTime();                              // Return delta time
    std::vector<intersectionData> computeCollisions(); // Compute all collisions in our world
    void resolveCollision(const intersectionData &a);
    void solveCollisions(std::vector<intersectionData> collisions, float deltatime); // Resolve collisions
    void integrate(float deltatime);                                                 // Apply physics calculations to the object
    void step(float deltaTime);                                                      // Per frame calculations
};