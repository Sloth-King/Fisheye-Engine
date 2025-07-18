

#include <vector>
#include <map>
#include "gameObject.h"

unsigned long  GameObjectData::next_id = 0;
std::set<GameObjectData*> GameObjectData::queuedForDeletion = std::set<GameObjectData*>();
std::mutex GameObjectData::queuedelete_mut = std::mutex();

void GameObjectData::addChild(GameObject && child){

    assert(("child is not an owning gameobject. ", child.isOwning()));
    auto p = child->id;
    GameObject ptr;

    if (child->parent != nullptr){
        ptr = std::move(child->parent->children[child->id]);
        child->parent->children.erase(p);
    } else {
        ptr = std::move(child);

    }
    children[p] = std::move(ptr);
    children[p]->parent = this;
    if (scene) children[p]->__enterScene(scene);


}
/*
void GameObjectData::addChild(GameObjectHandle && child){


    auto p = child->id;
    GameObjectHandle ptr = nullptr;

    if (child->parent != nullptr){
        ptr = std::move(child->parent->children[child->id]);
        child->parent->children.erase(p);
    } else {
        ptr = std::move(child);
    }

    children[p] = std::move(ptr);
    children[p]->parent = this;
    if (scene) children[p]->__enterScene(scene);

}*/

bool operator==(const GameObject& lhs, const GameObject& rhs)
{
    return lhs->getId() == rhs->getId();
}
