#include "engine/physics/components/components.h"
#include "engine/core/game/game.h"
#include "engine/core/scene/scene.h"

void C_Collider::_onEnterScene(){
    Scene& scene = getScene();
    if(scene.current_game){
        scene.current_game->physics_server.addObject(getOwner());
    }
};

void C_Collider::_onExitScene(){
    Scene& scene = getScene();
    if(scene.current_game){
        scene.current_game->physics_server.removeObject(getOwner());
    }
};

void C_RigidBody::_onEnterScene(){
    Scene& scene = getScene();
    if(scene.current_game){
        scene.current_game->physics_server.addObject(getOwner());
    }
};

void C_RigidBody::_onExitScene(){
    Scene& scene = getScene();
    if(scene.current_game){
        scene.current_game->physics_server.removeObject(getOwner());
    }
};

float C_RigidBody::inverseMass(){
    if(isStatic) return 0.0f;
    return 1.f/mass;
};

void C_RigidBody::applyForce(glm::vec3 force){
    auto physics = getScene().current_game->physics_server; // TODO maybe think a bit further than that ! 
    linear_velocity+=force * physics.getDeltaTime();
};

void C_RigidBody::applyImpulse(glm::vec3 impulse){
    linear_velocity+=impulse;
};

void C_RigidBody::applyAngularForce(glm::vec3 force){
    auto physics = getScene().current_game->physics_server; // TODO maybe think a bit further than that ! 
    angular_velocity+=force * physics.getDeltaTime();  
};

void C_RigidBody::applyAngularImpulse(glm::vec3 impulse){
    angular_velocity+=impulse;
};