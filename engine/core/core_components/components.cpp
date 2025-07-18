#include "components.h"
#include "engine/core/gameObject/gameObject.h"

const glm::mat4 & C_Transform::getLocalTransformationMatrix() const{

    if (!local_dirty){
        return local;
    }

    // if dirty

    glm::mat4 rotations = glm::rotate(glm::mat4(1.0f),
                            glm::radians(eulerRot.x),
                            glm::vec3(1.0f, 0.0f, 0.0f));
    rotations = glm::rotate(rotations,
                            glm::radians(eulerRot.y),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    rotations = glm::rotate(rotations,
                            glm::radians(eulerRot.z),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    local = glm::translate(glm::mat4(1.0f), pos) *
            rotations *
            glm::scale(glm::mat4(1.0f), scale);
    
    local_dirty = false;

    return local;

}   
const glm::mat4 & C_Transform::getGlobalTransformationMatrix() const{


    if (!global_dirty && !local_dirty){
        return global;
    }


    // if dirty

    GameObjectData * parent = getOwner()->parent;

    if (parent && parent->hasComponent<C_Transform>()){
        
        global = parent->getComponent<C_Transform>()->getGlobalTransformationMatrix() * getLocalTransformationMatrix();
    } else { // if our parent has no transform or we're the root, just assume we're the base transform.
        global = getLocalTransformationMatrix();
    }

    global_inv = glm::inverse(global);
    
    global_dirty = false;

    return global;

}

const glm::mat4 & C_Transform::getGlobalInverse() const{
    getGlobalTransformationMatrix();
    return global_inv;
}

// The getPos/Rot/Scale are all taken from there
// https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati=

const glm::vec3 & C_Transform::getGlobalPosition() const{
    static glm::vec3 globalPosition;
    glm::mat4 globalMatrix = getGlobalTransformationMatrix();
    globalPosition = glm::vec3(globalMatrix[3][0], globalMatrix[3][1], globalMatrix[3][2]);
    return globalPosition;
}

const glm::vec3 & C_Transform::getGlobalScale() const{
    static glm::vec3 globalScale;
    glm::mat4 globalMatrix = getGlobalTransformationMatrix();
    globalScale = glm::vec3(
        glm::length(glm::vec3(globalMatrix[0])),
        glm::length(glm::vec3(globalMatrix[1])),
        glm::length(glm::vec3(globalMatrix[2]))
    );
    return globalScale;
}

// FIXME idk if this is correct at all
const glm::vec3 & C_Transform::getGlobalRotation() const{
    static glm::vec3 globalRotation;
    glm::mat4 globalMatrix = getGlobalTransformationMatrix();
    glm::vec3 scale = getGlobalScale();  

    glm::mat3 rotationMatrix = glm::mat3(
        glm::vec3(globalMatrix[0][0]/scale.x , globalMatrix[0][1]/scale.y, globalMatrix[0][2]/scale.z),
        glm::vec3(globalMatrix[1][0]/scale.x , globalMatrix[1][1]/scale.y, globalMatrix[1][2]/scale.z),
        glm::vec3(globalMatrix[2][0]/scale.x , globalMatrix[2][1]/scale.y, globalMatrix[2][2]/scale.z)
    );

    globalRotation.x = glm::degrees(atan2(rotationMatrix[2][1], rotationMatrix[2][2]));
    globalRotation.y = glm::degrees(atan2(-rotationMatrix[2][0], sqrt(rotationMatrix[2][1] * rotationMatrix[2][1] + rotationMatrix[2][2] * rotationMatrix[2][2])));
    globalRotation.z = glm::degrees(atan2(rotationMatrix[1][0], rotationMatrix[0][0]));

    return globalRotation;
}

const glm::quat & C_Transform::getGlobalRotationQuat() const {
    static glm::quat globalRotationQuat;
    glm::mat4 globalMatrix = getGlobalTransformationMatrix();
    glm::vec3 scale = getGlobalScale();

    glm::mat3 rotationMatrix = glm::mat3(
        glm::vec3(globalMatrix[0][0] / scale.x, globalMatrix[0][1] / scale.y, globalMatrix[0][2] / scale.z),
        glm::vec3(globalMatrix[1][0] / scale.x, globalMatrix[1][1] / scale.y, globalMatrix[1][2] / scale.z),
        glm::vec3(globalMatrix[2][0] / scale.x, globalMatrix[2][1] / scale.y, globalMatrix[2][2] / scale.z)
    );

    globalRotationQuat = glm::quat_cast(rotationMatrix);
    return globalRotationQuat;
}


void C_Transform::setLocalDirty(){
    local_dirty = true;
    global_dirty = true;
    for (const auto & [ptr, object] : getOwner()->children){ // not ideal. waiting for a better idea. Maybe an apply_to_children(func f) method?
        GameObjectData & child = *object;
        if (child.hasComponent<C_Transform>()){
            child.getComponent<C_Transform>()->setGlobalDirty();
        }
    }
}

void C_Transform::setGlobalDirty(){
    global_dirty = true;

    for (const auto & [ptr, object] : getOwner()->children){ // not ideal. waiting for a better idea. Maybe an apply_to_children(func f) method?
        GameObjectData & child = *object;
        if (child.hasComponent<C_Transform>()){
            child.getComponent<C_Transform>()->setGlobalDirty();
        }
    }
}