#include "renderingServer.h"
#include "engine/core/game/game.h"
#include "engine/core/core_components/components.h"
#include "deferred.glsl"
#include <algorithm>

inline void limit_fps(int FPS){
    static double last_time = glfwGetTime();

    double current_time = glfwGetTime();
    double ms = 1.0 / FPS;
    while (current_time - last_time <= ms){
        current_time = glfwGetTime();
    }

    last_time = current_time;
}

inline void showFPS(GLFWwindow* window){

    static const float timeBetweenUpdates = 0.5;
    static double last_time = glfwGetTime();
    static unsigned int nbframes = 0;
    static const std::string windowTitle ("FPS - ");
    double current_time = glfwGetTime();

    nbframes++;
    
    if (current_time - last_time > timeBetweenUpdates){ // seconds
        glfwSetWindowTitle(window, (windowTitle + std::to_string((uint32_t)(nbframes/timeBetweenUpdates))).c_str());
        last_time = current_time;
        nbframes = 0;
    }
}

void RenderingServer::renderSceneForward() const noexcept {
    const Scene* curr_scene = &game->current_scene;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto* object: objects){
        object->renderForward();
    }
    curr_scene->environment.render(curr_scene->current_camera->getViewMatrix(), curr_scene->current_camera->getProjectionMatrix());

    glfwSwapBuffers(context);
}


void RenderingServer::renderSceneDeferred() const noexcept{

    // expects the context and game to be set, otherwise segfaults.
    geometryPass();
    lightPass();

    glfwSwapBuffers(context);

    showFPS(context);
}


void RenderingServer::geometryPass() const noexcept{

    const Scene* curr_scene = &game->current_scene;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gBuffer);
    glUseProgram(gShader);
    glDisable(GL_BLEND);


    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderMeshesDeferred();

    // for now, writes here, to the layout 4 (albedo)
    curr_scene->environment.render(curr_scene->current_camera->getViewMatrix(), curr_scene->current_camera->getProjectionMatrix());

}

// not really clean to put these three here but nothing else is ever gonna need em anyway
//maybe it's cleaner actually ?
GLuint _squareVBO;
GLuint _squareVAO;

std::array<Light, Light::MAX_NUMBER_OF_LIGHTS> lightsPassArray;
void RenderingServer::lightPass() const noexcept{

    const Scene* curr_scene = &game->current_scene;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glClear(GL_COLOR_BUFFER_BIT);
    

    glUseProgram(lightShader);
    
    glActiveTexture(GL_TEXTURE0+0); glBindTexture(GL_TEXTURE_2D, gPosition); glUniform1i(glGetUniformLocation(lightShader, "gPosition"), 0);
    glActiveTexture(GL_TEXTURE0+1); glBindTexture(GL_TEXTURE_2D, gNormal); glUniform1i(glGetUniformLocation(lightShader, "gNormal"), 1);
    glActiveTexture(GL_TEXTURE0+2); glBindTexture(GL_TEXTURE_2D, gAlbedoTex); glUniform1i(glGetUniformLocation(lightShader, "gAlbedoTex"), 2);
    glActiveTexture(GL_TEXTURE0+3); glBindTexture(GL_TEXTURE_2D, gRoughnessMetallicTex); glUniform1i(glGetUniformLocation(lightShader, "gRoughnessMetallicTex"), 3);
    glActiveTexture(GL_TEXTURE0+4); glBindTexture(GL_TEXTURE_2D, gDepth); glUniform1i(glGetUniformLocation(lightShader, "gDepth"), 4);

    glm::vec3 viewvector = curr_scene->current_camera->getForwardVector();
    glUniform3fv(glGetUniformLocation(lightShader, "view"), 1, &viewvector[0]);

    glm::mat4 VP = curr_scene->current_camera->getVP();
    glUniformMatrix4fv(glGetUniformLocation(lightShader, "VP"), 1, GL_FALSE, &VP[0][0]);

    // light setup
    int currentLightCount = 0;

    // on prend les N premières lights qui sont assez proches du joueur

    for (C_Light* light: lights){

        Light & lightStruct = light->light;

        if (light->getOwner()->hasComponent<C_Transform>()){
            lightStruct._pos = light->getOwner()->getComponent<C_Transform>()->getGlobalPosition();
        } else {
            lightStruct._pos = glm::vec3(0, 0, 0);
        }

        if (glm::distance(
                lightStruct._pos,
                curr_scene->current_camera->getGlobalPosition()
            ) <= lightStruct.radius){

            lightsPassArray[currentLightCount] = lightStruct; // copy

            ++currentLightCount;
        }

        if (currentLightCount > Light::MAX_NUMBER_OF_LIGHTS) break;
    }

    glUniform1i(glGetUniformLocation(lightShader, "currentLightCount"), currentLightCount);

    glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, lightsPassArray.size() * sizeof(Light), lightsPassArray.data());
    glBindBuffer(GL_UNIFORM_BUFFER,0);

    glBindVertexArray(_squareVAO);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}


float squareVertices[] = {
    -1, -1,
    1, -1,
    1, 1,

    -1, -1,
    1, 1,
    -1, 1};

// https://learnopengl.com/Advanced-Lighting/Deferred-Shading
void RenderingServer::setupBuffers(){
    /*
    layout (location = 0) out vec3 __Position__;
    layout (location = 1) out vec3 __Normal__;
    layout (location = 3) out vec3 __Tangent__;
    layout (location = 4) out vec3 __Bitangent__;
    layout (location = 5) out vec4 __Albedo__;
    layout (location = 6) out float __Depth__;
    
    */

    
    glDeleteTextures(9, &gPosition);
    glDeleteFramebuffers(1, &gBuffer);

    
    if (!gShader){
        gShader = loadShaders(vertex_deferred, fragment_deferred);
    }

    if (!lightShader){

        Utils::replaceInString(
            fragment_PBR_deferred,
            "PLACEHOLDER_num_lights",
            std::to_string(Light::MAX_NUMBER_OF_LIGHTS)
        );
        lightShader = loadShaders(PBR_vertex_deferred, fragment_PBR_deferred);

        // setting up screen plane for second pass
        glGenVertexArrays(1, &_squareVAO);
        glBindVertexArray(_squareVAO);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &_squareVBO);
        glBindBuffer(GL_ARRAY_BUFFER, _squareVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), &squareVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);

        // setting up light array uniform

        // recuperer l'identifiant du groupe d'uniforms frameData
        GLuint block= glGetUniformBlockIndex(lightShader, "lightsUniform");
        // associer l'indice 0 a frameData
        glUniformBlockBinding(lightShader, block, 0);
        
        // creer et initialiser le buffer // https://community.khronos.org/t/uniform-buffer-objects-dynamic-sized-arrays-and-lights/70415
        glGenBuffers(1,&lightsUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
        glBufferData(GL_UNIFORM_BUFFER, Light::MAX_NUMBER_OF_LIGHTS * sizeof(Light), lightsPassArray.data(), GL_DYNAMIC_DRAW); // this allocates space for the UBO. 

        // selectionner un buffer existant pour affecter une valeur a tous les uniforms du groupe simpleData
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO);

    }

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    auto SCR_WIDTH = game->settings.windowWidth;
    auto SCR_HEIGHT = game->settings.windowHeight;
    
    // - position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    
    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    
    // - albedo color buffer
    glGenTextures(1, &gAlbedoTex);
    glBindTexture(GL_TEXTURE_2D, gAlbedoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoTex, 0);


    // - roughness color buffer
    glGenTextures(1, &gRoughnessMetallicTex);
    glBindTexture(GL_TEXTURE_2D, gRoughnessMetallicTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, SCR_WIDTH, SCR_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughnessMetallicTex, 0);

    // - depth color buffer
    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);


    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);



    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glFinish();
}