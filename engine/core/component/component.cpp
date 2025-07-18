#include "component.h"
#include "engine/core/gameObject/gameObject.h"
#include <stdio.h>
#include "engine/core/scene/scene.h"
#include "engine/core/game/game.h"

GameObjectData* Component::getOwner() const noexcept{

    return owner;
}

Scene& Component::getScene(){
    assert(("getScene called when not in scene.", owner->getScene()));
    return *owner->getScene();
}

void Component::_onEnterScene() {}
void Component::_onExitScene() {}

void Component::_onUpdate(float deltaTime) {};
void Component::_onPhysicsUpdate(float deltaTime) {};
void Component::_onLateUpdate(float deltaTime) {};