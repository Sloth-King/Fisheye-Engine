#pragma once


#include "engine/core/component/component.h"

#include "engine/render/mesh/mesh.h"
#include "engine/render/camera/camera.h"
#include "engine/render/light/light.h"

class C_Mesh: public Component {
public:
    
    Mesh mesh;

    //C_Mesh(Mesh & mesh): mesh(mesh) {}
    C_Mesh() = default;
    
    void renderForward();
    void renderDeferred(GLuint gShader);

    virtual void _onEnterScene() override;

    virtual void _onExitScene() override;

};

class C_Camera: public Component{
public:
    Camera camera;
    glm::vec3 offset = glm::vec3(0.0,0.0,0.0);

    C_Camera() = default;

    virtual void _onLateUpdate(float deltaTime) override;   

    virtual void _onEnterScene() override;
};

class C_Light: public Component{
public:
    Light light;

    virtual void _onEnterScene() override;

    virtual void _onExitScene() override;
};

