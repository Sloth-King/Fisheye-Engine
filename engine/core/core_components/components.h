#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/core/component/component.h"
#include <iostream>

class C_Transform: public Component {

private:

    mutable glm::mat4 local = glm::mat4(1.0f);
    mutable glm::mat4 global = glm::mat4(1.0f);
    mutable glm::mat4 global_inv = glm::mat4(1.0f);

    mutable bool local_dirty = false;
    mutable bool global_dirty = true;

    glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 eulerRot = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    void setLocalDirty();

    void setGlobalDirty();

public:
    const glm::mat4 & getLocalTransformationMatrix() const;
    const glm::mat4 & getGlobalTransformationMatrix() const;
    const glm::mat4 & getGlobalInverse() const;

    const glm::vec3 & getGlobalPosition() const;
    const glm::vec3 & getGlobalScale() const;
    const glm::vec3 & getGlobalRotation() const;
    const glm::quat & getGlobalRotationQuat() const;


    // local funcs
    void move(glm::vec3 speed_per_coord){
        pos += speed_per_coord;

        setLocalDirty();
    }

    void rotate(glm::vec3 speed_per_coord){
        eulerRot += speed_per_coord;
        setLocalDirty();
    }

    void setPosition(glm::vec3 newpos){
        pos = newpos;
        setLocalDirty();
    }
    void setScale(glm::vec3 newScale){
        scale = newScale;
        setLocalDirty();
    }
    void setRotation(glm::vec3 newRot){
        eulerRot = newRot;
        setLocalDirty();
    }

    glm::vec3 getPosition(){
        return pos;
    }

    glm::vec3 getScale(){
        return scale;
    }

    // global funcs TDOO
    
    void moveGlobal(glm::vec3 speed_per_coord){
        pos += glm::vec3(getGlobalInverse() * glm::vec4(speed_per_coord, 0));
        // std::cout << " pos: " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
        setLocalDirty(); // c'est la local qui bouge même si on bouge dans l'espace global !
    }

    void rotateGlobal(glm::vec3 speed_per_coord){
        eulerRot += glm::vec3(getGlobalInverse() * glm::vec4(speed_per_coord, 0));
        setLocalDirty();
    }

    void setPositionGlobal(glm::vec3 newpos){
        pos = glm::vec3(getGlobalInverse() * glm::vec4(newpos, 0));
        setLocalDirty();
    }
    void setScaleGlobal(glm::vec3 newScale){
        scale = glm::vec3(getGlobalInverse() * glm::vec4(newScale, 0));
        setLocalDirty();
    }
    void setRotationGlobal(glm::vec3 newRot){
        eulerRot = glm::vec3(getGlobalInverse() * glm::vec4(newRot, 0));
        setLocalDirty();
    }

    void printGlobal(){
        auto g = getGlobalTransformationMatrix();

        for (int i = 0; i < 4; ++i){
            std::cout << "| " << g[0][i] << ", " << g[1][i] << ", " << g[2][i] << ", " << g[3][i] << " |" << std::endl;
        }
    }
    void printLocal(){
        auto g = getLocalTransformationMatrix();

        for (int i = 0; i < 4; ++i){
            std::cout << "| " << g[0][i] << ", " << g[1][i] << ", " << g[2][i] << ", " << g[3][i] << " |" << std::endl;
        }
    }
};

