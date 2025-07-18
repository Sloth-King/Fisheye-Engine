#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>
#include <functional>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/utils/utils.h"

enum BIND_TYPE {
    BINDTYPE_KEYBOARD,
    BINDTYPE_MOUSE
};


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


struct KeySpec {
    BIND_TYPE type;

    unsigned short inputData; // either the key if type is keyboard, or the mouse button
    
    // not ideal but enough. Used for the std::map key ordering
    bool operator<(const KeySpec& other) const{
        if (type < other.type) return true;
        if (type > other.type) return false;
        return inputData < other.inputData;
    }
};

struct InputEvent {

    const std::string name;

    const bool pressed; // or released

    InputEvent(std::string p_name, bool p_pressed)
        : name(p_name),
        pressed(p_pressed)
        {}

    inline bool isReleased() const noexcept { return !pressed;}

    inline bool isPressed() const noexcept { return pressed;}

};

inline bool operator==(const InputEvent& lhs, const std::string& rhs)
{
    return lhs.name == rhs;
}

inline bool operator==(const std::string& lhs, const InputEvent& rhs)
{
    return rhs == lhs;
}



using inputCallbackPointer_t = std::function<void(const InputEvent &)>;

class Input {

friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


    static std::map<KeySpec, std::string> bindToName;

    static std::map<std::string, bool> nameToPressed;

    static std::vector< inputCallbackPointer_t > subscribers;

    static void sendEvent(InputEvent e){
        for (auto sub: subscribers){
            sub(e);
        }
    }

public:
    static void init(){ // to be called after window setup !
        glfwSetKeyCallback(glfwGetCurrentContext(), key_callback);
        glfwSetMouseButtonCallback(glfwGetCurrentContext(), mouse_button_callback);
    }

    static void addKeybind(std::string name, KeySpec key){
        bindToName[key] = name;
        nameToPressed[name] = false;
    }

    static void addInputListener(inputCallbackPointer_t f){
        subscribers.push_back(f);
    }

    static inline bool isInputPressed(const std::string & name){
        return nameToPressed[name];
    }

    static inline bool isInputReleased(const std::string & name){
        return !nameToPressed[name];
    }
};