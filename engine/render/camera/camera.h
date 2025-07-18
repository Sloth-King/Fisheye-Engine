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

class Camera{

protected:
    glm::mat4 projection;
    bool rotating = false;
public:
    glm::mat4 transform; // !! is actually the inverse view matrix

    GLFWwindow* window;

    float width = 500;
    float height = 500;

    Camera(){
        transform = glm::mat4(1.0f);

        projection = glm::perspective(
            glm::radians(45.0f),
            width / height,
            0.1f,
            300.0f
        );
    }

    void resize(int u, int v){
        projection = glm::perspective(
            glm::radians(60.0f),
            (float)u / (float)v,
            0.1f,
            300.0f
        );

    }

    inline glm::mat4 getProjectionMatrix(){
        return projection;
    }

    inline glm::mat4 getViewMatrix(){
        return glm::inverse(transform);
    }

    inline glm::vec3 getGlobalPosition(){
        return glm::vec3(transform[3]); 
    }

    inline glm::mat4 getVP(){
        //view = glm::inverse(getViewMatrix());
        return getProjectionMatrix() * getViewMatrix();
    }

    inline glm::vec3 getForwardVector(){
        //view = glm::inverse(getViewMatrix());
        return glm::vec3(transform[2][0], transform[2][1], transform[2][2]);
    }
    inline glm::vec3 getUpVector(){
        //view = glm::inverse(getViewMatrix());
        return glm::vec3(transform[1][0], transform[1][1], transform[1][2]);
    }
    inline glm::vec3 getRightVector(){
        //view = glm::inverse(getViewMatrix());
        return glm::vec3(transform[0][0], transform[0][1], transform[0][2]);
    }
};

class DebugCamera: public Camera{
public:
    float mouseSpeed = 2.0;
    float speed = 4.0;

    void computeMatricesFromInputs(){
        if (!glfwGetCurrentContext()) return;

        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();

        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);

        // Get mouse position
        static double xpos_last, ypos_last;
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

        // Reset mouse position for next frame
        //glfwSetCursorPos(window, 1024/2, 768/2);

        // Compute new orientation

        if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){

            float horizontalAngle = mouseSpeed * (xpos - xpos_last) / 1000.0;
            float verticalAngle   = mouseSpeed * (ypos - ypos_last) / 1000.0;

            glm::vec3 t0 = glm::vec3(transform[0]);

            // in global space
            // transform = glm::rotate(glm::mat4(1.0), horizontalAngle, glm::vec3(transform[1])) * transform;
            // transform = glm::rotate(glm::mat4(1.0), verticalAngle, t0) * transform;

            // in local space
            transform = glm::rotate(transform, -horizontalAngle, glm::vec3(0.0, 1.0, 0.0));
            transform =  glm::rotate(transform, -verticalAngle, glm::vec3(1.0, 0.0, 0.0));


            glm::vec3 global_up = glm::vec3(0, 1, 0);
            // transform[2] = glm::normalize(transform[2]);
            // transform[0] = glm::vec4(glm::normalize(glm::cross(glm::vec3(transform[2]), global_up)), 0.0);
            // transform[1] = glm::vec4(glm::cross(
            //     glm::vec3(transform[0]),
            //     glm::vec3(transform[2])
            // ), 0.0);
            // float ang = -glm::acos(glm::dot(global_up, glm::normalize(glm::vec3(transform[1]))));
            // transform = glm::rotate(transform, ang, glm::vec3(transform[2]));
            
            glm::mat3 test = glm::mat3(
                glm::vec3(transform[1]),
                glm::vec3(global_up),
                glm::vec3(transform[2])
            );

        }
        lastTime = currentTime;

        xpos_last = xpos;
        ypos_last = ypos;

        // Move forward

        if (glfwGetKey( glfwGetCurrentContext(), GLFW_KEY_UP ) == GLFW_PRESS){
            transform = glm::translate(
                transform,
                glm::vec3(0.0, 0.0, -1.0) * deltaTime * speed
            );
        }
        // Move backward
        if (glfwGetKey( glfwGetCurrentContext(), GLFW_KEY_DOWN ) == GLFW_PRESS){
            transform = glm::translate(
                transform,
                glm::vec3(0.0, 0.0, 1.0) * deltaTime * speed
            );
        }
        // Strafe right
        if (glfwGetKey( glfwGetCurrentContext(), GLFW_KEY_RIGHT ) == GLFW_PRESS){
            transform = glm::translate(
                transform,
                glm::vec3(1.0, 0.0, 0.0) * deltaTime * speed
            );
        }
        // Strafe left
        if (glfwGetKey( glfwGetCurrentContext(), GLFW_KEY_LEFT ) == GLFW_PRESS){
            transform = glm::translate(
                transform,
                glm::vec3(-1.0, 0.0, 0) * deltaTime * speed
            );
        }
        // Activate rotation
        if (glfwGetKey( glfwGetCurrentContext(), GLFW_KEY_R ) == GLFW_PRESS){
            rotating = !rotating;
        }
	}
};