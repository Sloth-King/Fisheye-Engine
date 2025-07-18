#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/core/component/component.h"
#include "engine/physics/collider/collider.h"
#include <iostream>

// Physics and collision
class C_RigidBody : public Component
{

public:
    glm::vec3 linear_velocity;      // Velocity for xyz directions
    glm::vec3 angular_velocity;     // Velocity for when we handle rotations
    glm::vec3 acceleration;         // Acceleration
    float damping = 0.99f;          // Damping to simulate drag naively
    float mass = 1.0f;              // Mass of our object (maybe switch to inverse mass)
    float restitution = 0.5;        // Pretty much bounciness (i hope no phycicist reads this)
    bool isStatic = false;          // Decide if the object is static or no (infinite mass obj, like the floor or a wall)
    float gravityScale = 1.0f;

    virtual void _onEnterScene() override;
    virtual void _onExitScene() override;

    // Constructor
    C_RigidBody(glm::vec3 initial_velocity = glm::vec3(0.0f), glm::vec3 initial_angular_velocity = glm::vec3(0.0f), glm::vec3 initial_acceleration = glm::vec3(0.0f),
                float damping_coef = 0.99f, float m = 1.0f, float restitution_coef = 0.5f, bool isStatic = false, float gravityScale = 1.0f)
        : linear_velocity(initial_velocity), angular_velocity(initial_angular_velocity), damping(damping_coef), mass(m), restitution(restitution_coef), isStatic(isStatic), gravityScale(gravityScale) {}

    float inverseMass();

    void applyForce(glm::vec3 force);
    void applyImpulse(glm::vec3 impulse);
    void applyAngularForce(glm::vec3 force);
    void applyAngularImpulse(glm::vec3 impulse);


    void setVelocity(glm::vec3 velocity)
    {
        linear_velocity = velocity;
    }
};

union Colliders
{
    Collider base;
    SphereCollider sphere;
    PlaneCollider plane;
    CubeCollider cube;
    ChunkCollider chunk;

    Colliders() : base() {}
};

// Collision shape
class C_Collider : public Component
{
public:
    Colliders collider;

    C_Collider()
    {
        collider.base = Collider(BASE);
    }

    virtual void _onEnterScene() override;
    virtual void _onExitScene() override;
};
