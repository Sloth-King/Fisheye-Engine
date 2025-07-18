#include "input.h"



std::map<KeySpec, std::string> Input::bindToName = std::map<KeySpec, std::string>();
std::map<std::string, bool> Input::nameToPressed = std::map<std::string, bool>();
std::vector< inputCallbackPointer_t > Input::subscribers = std::vector< inputCallbackPointer_t >();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    KeySpec keyspec(BINDTYPE_KEYBOARD, key);

    if (!Input::bindToName.contains(keyspec)) return;

    if (action == GLFW_PRESS){

        const std::string& name = Input::bindToName[keyspec];


        // just in case
        if (Input::nameToPressed[name]) return;

        Input::nameToPressed[name] = true;

        InputEvent e(
            Input::bindToName[keyspec],
            true
            );

        Input::sendEvent(e);

    } else if (action == GLFW_RELEASE) {
        const std::string& name = Input::bindToName[keyspec];

        // just in case
        if (!Input::nameToPressed[name]) return;

        Input::nameToPressed[name] = false;

        InputEvent e(
            Input::bindToName[keyspec],
            false
            );

        Input::sendEvent(e);
    }
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    KeySpec keyspec(BINDTYPE_MOUSE, button);

    if (!Input::bindToName.contains(keyspec)) return;


    if (action == GLFW_PRESS){

        const std::string& name = Input::bindToName[keyspec];

        Input::nameToPressed[name] = true;

        InputEvent e(
            Input::bindToName[keyspec],
            true
            );

        Input::sendEvent(e);



    } else if (action == GLFW_RELEASE) {
        const std::string& name = Input::bindToName[keyspec];

        Input::nameToPressed[name] = false;

        InputEvent e(
            Input::bindToName[keyspec],
            false
        );

        Input::sendEvent(e);
    }
}