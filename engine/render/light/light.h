#pragma once
#include <glm/glm.hpp>


class RenderingServer;
class Light{
    friend RenderingServer;

    // set by the renderingserver in the copied struct to pass to the gpu
    glm::vec3 _pos;

public:
    static const uint MAX_NUMBER_OF_LIGHTS = 256;

    // explicit type is important for passing to gpu
    enum LIGHT_TYPE : signed int {LIGHT_NONE, LIGHT_POINT, LIGHT_TYPE_MAX};

    LIGHT_TYPE type = LIGHT_NONE;

    glm::vec3 color = glm::vec3(1.0);
    float intensity = 1.0;

    // careful : not transformed for now
    glm::vec3 dir = glm::vec3(0.0, -1.0, 0.0);

    float radius = 15.0;
};