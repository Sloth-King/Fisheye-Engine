

#pragma once
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <typeindex>
#include <cassert>
#include <variant>
#include <set>
#include <mutex>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/utils/utils.h"

#include "engine/core/component/component.h"

#include "engine/core/resource/resource.h"


template<typename T> // https://stackoverflow.com/questions/65969224/c-templates-and-inheritance-can-templates-be-more-selective
concept DerivedFromComponent = std::is_base_of_v<Component,T>;

class GameObjectData;

using GameObjectHandle = std::unique_ptr<GameObjectData>;

// kinda a handle object to wrap around the owning-or-not pointer
class GameObject{

    bool owning = true; // if it's owning the data

    std::variant< GameObjectData*, GameObjectHandle > _ptr;

    inline GameObjectData* getData() const {
        return (owning) ? std::get<1>(_ptr).get() : std::get<0>(_ptr);
    }

public:

    GameObject()
        :_ptr(std::make_unique<GameObjectData>())
        {}

    GameObject& operator=(const GameObject & other){
        owning = false;
        _ptr = other.getData();
        return *this;
    }
    GameObject(const GameObject & other){
        *this = other;
    }

    explicit operator bool() const {
        return static_cast<bool>(getData());
    }

    GameObject& operator=(GameObject&& other){
        //TODO do we allow moving from nonowning ptr?
        if (&other == this) return *this;

        assert(("Gameobject must be owning in order to be moved from. ", other.isOwning()));

        _ptr = std::move(std::get<1>(other._ptr));
        other._ptr = std::get<1>(_ptr).get();
        owning = true;
        other.owning = false;

        return *this;
    }

    explicit GameObject(GameObject && other){
        *this = std::move(other);
    }

    inline bool isOwning() const {return owning;}
    
    inline GameObjectData* operator -> () const{
        return getData();
    }
    inline GameObjectData& operator * () const{
        return *getData();
    }



    GameObject copy() const { //TODO replace with a real, deep copy of the object and rename this one
        return *this;
    }
    /*
    template <DerivedFromComponent T>
    inline T* addComponent() const{
        return getData()->addComponent<T>();
    }
    inline void addChild(GameObject && object){
        getData()->addChild(std::move(object));
    }
    */
};


class Scene;
class Game;
class GameObjectData{
    friend Scene;
    friend Game;
    friend GameObject;
    friend GameObjectHandle std::make_unique<GameObjectData>(); // idk why it precisely needs this but it does .... ???

    static unsigned long next_id;

    static std::set<GameObjectData*> queuedForDeletion;

    unsigned long id = 0;

    GameObjectData(): id(next_id++) {};

    void deleteChild(unsigned long id){
        children.erase(id); // gameobjects are always master of their children
    }

    static std::mutex queuedelete_mut; // important to keep ordered
public:
    GameObjectData * parent = nullptr; // null in case it's root of a scene
    
    std::map< unsigned long, GameObject > children = std::map< unsigned long, GameObject >();

    std::map<std::type_index, std::unique_ptr<Component> > components = std::map<std::type_index, std::unique_ptr<Component> >();

    Scene * scene = nullptr;

    
    GameObjectData( GameObjectData&& rcOther )
    : children( std::move(rcOther.children) )
    , components( std::move(rcOther.components) )
    , scene(rcOther.scene)
    , id(rcOther.id)
    {
        for (const auto & [id, owning_ptr] : children)
            owning_ptr->parent = this;
        
        for (const auto & [ti, comp] : components)
            comp->owner = this;
    }
    

    bool hidden = false;
    // bool deactivated = false;

    void addChild(GameObject && child);

    bool isRoot() const { return (bool)parent; };

    Scene * getScene() const {
        assert(scene != nullptr);
        return scene;
    }

    void __enterScene(Scene * s){
        scene = s;
        for (const auto & [id, owning_ptr] : children)
            owning_ptr->__enterScene(s);

        for (const auto & [ti, comp] : components)
            comp->_onEnterScene();
    }

    void __exitScene(){
        for (const auto & [id, owning_ptr] : children)
            owning_ptr->__exitScene();

        for (const auto & [ti, comp] : components)
            comp->_onExitScene();
        
        scene = nullptr;
    }

    unsigned long getId() const {
        return id;
    }

    void queueDelete(){

        std::scoped_lock lock(queuedelete_mut);

        if (parent) // dont delete objects outside lol
            queuedForDeletion.insert(this);
    }
    
    ~GameObjectData(){
        if (scene)
            for (const auto & [ti, comp] : components)
                comp->_onExitScene();
    }


    template <DerivedFromComponent T>
    bool hasComponent() const {
        return components.contains(std::type_index(typeid(T)));
    }

    template <DerivedFromComponent T>
    T* getComponent(){
        return dynamic_cast<T*>(components[std::type_index(typeid(T))].get());
    }
    /*
    void addComponent(Component && c){
        c.owner = this;
        components[std::type_index(typeid(c))] = std::make_unique<Component>(std::move(c));
    }*/

    // copies the component instead
    template <DerivedFromComponent T>
    T* addComponent(){
        components[std::type_index(typeid(T))] = std::make_unique<T>();
        components[std::type_index(typeid(T))]->owner = this;
        return dynamic_cast<T*>(components[std::type_index(typeid(T))].get());
    }

    void __engineUpdate(float deltaTime){
        
        for (const auto & [id, owning_ptr] : children)
            owning_ptr->__engineUpdate(deltaTime);
        
        for (const auto & [ti, comp] : components)
            comp->_onUpdate(deltaTime);

    }

    void __enginePhysicsUpdate(float deltaTime){

        for (const auto & [id, owning_ptr] : children)
            owning_ptr->__enginePhysicsUpdate(deltaTime);

        for (const auto & [ti, comp] : components)
            comp->_onPhysicsUpdate(deltaTime);
    }

    void __lateUpdate(float deltaTime){
        
        for (const auto & [id, owning_ptr] : children)
            owning_ptr->__lateUpdate(deltaTime);
        
        for (const auto & [ti, comp] : components)
            comp->_onLateUpdate(deltaTime);

    }
};

bool operator==(const GameObject& lhs, const GameObject& rhs);