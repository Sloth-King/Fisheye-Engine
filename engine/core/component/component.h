#pragma once

#include <map>
#include <memory>
#include <typeindex> // for rtti use
// forward declaration
class GameObjectData;
class Scene;

class Component {
    friend GameObjectData;
protected:
    GameObjectData* owner;
public:

    Component(Component && v) = default;
    Component(const Component & v) = default;

    GameObjectData* getOwner() const noexcept;

    Scene& getScene();
    
    Component() = default;

    virtual void _onEnterScene();
    virtual void _onExitScene();

    virtual void _onUpdate(float deltaTime);
    virtual void _onPhysicsUpdate(float deltaTime);
    virtual void _onLateUpdate(float deltaTime);

};