#pragma once
#include "engine/includes/core.h"

class C_PlayerController : public Component
{
public:
    float movement_speed = 1.7;
    float mouseSpeed = 100;
    bool _isMouseHidden = false;

    void inputCallback(const InputEvent &e)
    {
        /*
        if (e == "move_right"){
            std::cout << "move_right" << std::endl;
        }
        else if (e == "move_left"){
            std::cout << "move_left" << std::endl;
        }
        else if (e == "move_forward"){
            std::cout << "move_forward" << std::endl;
        }
        else if (e == "move_backwards"){
            std::cout << "move_backwards" << std::endl;
        }
        else if (e == "move_up"){
            std::cout << "move_up" << std::endl;
        }
        else if (e == "move_down"){
            std::cout << "move_down" << std::endl;
        }
        */
    }

    C_PlayerController()
    {
        Input::addInputListener(
            [&/*capture ambient object ref*/](const InputEvent &e)
            {
                inputCallback(e);
            });
    }

    virtual void _onUpdate(float delta) override
    {
        if (!getOwner()->hasComponent<C_Transform>() || !getOwner()->hasComponent<C_RigidBody>())
            return;

        auto *transform = getOwner()->getComponent<C_Transform>();
        auto *body = getOwner()->getComponent<C_RigidBody>();

        if (getOwner()->hasComponent<C_Camera>())
        {
            auto *cam = getOwner()->getComponent<C_Camera>();
            
            delta *= movement_speed;

            glm::vec3 linearForce(0.0f);

            if (Input::isInputPressed("move_right"))
            {
                linearForce += cam->camera.getRightVector();
            }
            if (Input::isInputPressed("move_left"))
            {
                linearForce -= cam->camera.getRightVector();
            }
            if (Input::isInputPressed("move_forward"))
            {
                linearForce -= cam->camera.getForwardVector();
            }
            if (Input::isInputPressed("move_backwards"))
            {
                linearForce += cam->camera.getForwardVector();
            }
            if (Input::isInputPressed("move_up"))
            {
                linearForce += cam->camera.getUpVector();
            }
            if (Input::isInputPressed("move_down"))
            {
                linearForce -= cam->camera.getUpVector();
            }

            // Utils::print("linear force", linearForce);

            body->applyForce(delta * linearForce);

            // Mouse movement

            {

                static double xpos_last, ypos_last;
                double xpos, ypos;
                if (!_isMouseHidden)
                {
                    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
                    xpos_last = xpos;
                    ypos_last = ypos;
                    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    _isMouseHidden = true;
                }

        
                glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

                // Reset mouse position for next frame
                // glfwSetCursorPos(glfwGetCurrentContext(), 1024/2, 768/2);

                // Compute new orientation

                float horizontalAngle = mouseSpeed * (xpos - xpos_last) / 1000.0;
                float verticalAngle = mouseSpeed * (ypos - ypos_last) / 1000.0;

                // in local space
                // transform->rotate(-horizontalAngle * glm::vec3(0.0, 1.0, 0.0));
                // transform->rotate(-verticalAngle * glm::vec3(0.0, 0.0, 1.0));

                body->applyAngularForce(-horizontalAngle * glm::vec3(0.0, 1.0, 0.0));
                body->applyAngularForce(-verticalAngle * glm::vec3(1.0, 0.0, 0.0));

                xpos_last = xpos;
                ypos_last = ypos;
            }
        }
        else
        {
            delta *= movement_speed;

            glm::vec3 linearForce(0.0f);

            if (Input::isInputPressed("move_right"))
            {
                linearForce += glm::vec3(1, 0, 0);
            }
            if (Input::isInputPressed("move_left"))
            {
                linearForce += glm::vec3(-1, 0, 0);
            }
            if (Input::isInputPressed("move_forward"))
            {
                linearForce += glm::vec3(0, 0, -1);
            }
            if (Input::isInputPressed("move_backwards"))
            {
                linearForce += glm::vec3(0, 0, 1);
            }
            if (Input::isInputPressed("move_up"))
            {
                linearForce += glm::vec3(0, 1, 0);
            }
            if (Input::isInputPressed("move_down"))
            {
                linearForce += glm::vec3(0, -1, 0);
            }

            body->applyForce(linearForce * delta);
        }
    };
};