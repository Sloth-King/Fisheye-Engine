

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <thread>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/utils/utils.h"
#include "engine/core/scene/scene.h"
#include "engine/physics/physics_server/physics_server.h"
#include "engine/render/renderingServer/renderingServer.h"

struct GameSettings {
    int windowWidth;
    int windowHeight;
    float windowAspect;
    float fov;

    bool VSync = false;
};

class Game {

friend void handleWindowResizedCallback(GLFWwindow* window, int width, int height);
protected:

    void renderloop();

    std::thread render_thread;

    GLFWwindow* window;
    bool running = false;

    void handleWindowResized();


public:
    // updated automatically when a camera is added or the window is changed
    GameSettings settings;

    Game(){
        current_scene.current_game = this;
    }

    inline bool isRunning() const { return running;}

    GLFWwindow* getWindow() {
        return window;
    }

    void setCurrentCamera(Camera& c){
        c.resize(settings.windowWidth, settings.windowHeight);
        current_scene.setCurrentCamera(c);
    }

    Scene current_scene;
    PhysicsServer physics_server;
    RenderingServer renderingServer;

    Scene * setScene(Scene && scene){ // TODO call enter and exitscene if comp added at runtime
        current_scene.__unloaded();
        current_scene = std::move(scene);
        current_scene.current_game = this;
        
        if (current_scene.current_camera) current_scene.current_camera->resize(settings.windowWidth, settings.windowHeight);
        current_scene.__loaded();
        return &current_scene;
    }

    void start();

    void init();

    void renderUpdate();

    void physicsUpdate();

    void stop();

    void waitGameStopped(){
        while (running) {}
    }
};