#pragma once

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include "engine/core/resourceLoader/resourceLoader.h"

#include "engine/core/gameObject/gameObject.h"
#include "engine/utils/utils.h"
#include "engine/render/camera/camera.h"
#include "engine/render/environment/environment.h"
class Game;
class Scene{

    DebugCamera debugCamera;
//private:
public:
    Camera* current_camera = nullptr;

    Game * current_game = nullptr;

    GameObject root;

public:

    Environment environment;

    Scene() = default;

    Scene& operator=(Scene&& other) noexcept{
        // Make sure we're not assigning to ourselves
        if (this != &other) {


            current_camera = other.current_camera;
            //current_game = other.current_game;
            root = std::move(other.root);

            other.current_camera = nullptr;
            other.current_game = nullptr;
        }
        return *this;
    }

    Scene(Scene && other){
        *this = std::move(other);
    }

    Camera& getCurrentCamera(){
        return *current_camera;
    }
    void setCurrentCamera(Camera& c){
        current_camera = &c;
    }

    void setRoot(GameObject && g){ // TODO async in regards to the updateloop for now, probably not a good idea but we'll manage that later
        if (root) root->__exitScene();

        root = std::move(g);
        root->__enterScene(this);
    }
    
    void __engineUpdate(float deltaTime){

        if (!current_camera){
            std::cout << "[SCENE] No camera detected. Defaulting to degubCamera." << std::endl;
            setCurrentCamera(debugCamera);
        }
        
        if (root){
            if (current_camera == &debugCamera) debugCamera.computeMatricesFromInputs();
            root->__engineUpdate(deltaTime);
        }
    }
    void __engineLateUpdate(float deltaTime){

        if (root){
            root->__lateUpdate(deltaTime);
        }
    }

    void __enginePhysicsUpdate(float deltaTime){
        if (root)
            root->__enginePhysicsUpdate(deltaTime);
    }

    void __loaded(){
        if (root) root->__enterScene(this);
    }

    void __unloaded(){
        if (root) root->__exitScene();
    }
};