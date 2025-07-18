
#include "game.h"
#include "core/inputs/input.h"


// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <thread>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>




void Game::handleWindowResized(){
    renderingServer.setupBuffers();
    glViewport(0, 0, settings.windowWidth, settings.windowHeight);
    if (current_scene.current_camera){
        current_scene.current_camera->resize(settings.windowWidth, settings.windowHeight); // update projection matrix
    }
}

Game* current_game; // for this, because methods cant be glfw callbacks
bool needResizeWindow = false;
void handleWindowResizedCallback(GLFWwindow* window, int width, int height){
    current_game->settings.windowWidth = width;
    current_game->settings.windowHeight = height;
    needResizeWindow = true;
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    return ;//
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void activateGLErrors(){
    glEnable(GL_DEBUG_OUTPUT);        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

void Game::init()
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return ;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( settings.windowWidth, settings.windowHeight, "Game", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window." );
        getchar();
        glfwTerminate();
        return ;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return ;
    }


    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(0.0, 0.0, 0.0, 1.0);


    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable (GL_PROGRAM_POINT_SIZE);
    glDepthFunc(GL_LEQUAL);

    glfwSwapInterval(settings.VSync);

    current_game = this;

    glfwSetFramebufferSizeCallback(window, handleWindowResizedCallback);
    glEnable(GL_CULL_FACE); 

    

    renderingServer.setContext(window);
    renderingServer.setGame(this);
    renderingServer.setupBuffers();

    Input::init();

}

void Game::renderloop(){
    glfwMakeContextCurrent(window);
    while (running && !glfwWindowShouldClose(window)) renderUpdate();
}

void Game::start(){
    running = true;
    /*
    render_thread = std::thread(
        &Game::renderloop,
        this
    );
    */
    Game::renderloop();
}

float deltaTime = 0.0f;
double lastFrame = 0.0f;
void Game::renderUpdate(){
    // Measure speed
    // per-frame time logic
    // --------------------
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwPollEvents();

    if (needResizeWindow){
        needResizeWindow = false;
        handleWindowResized();
    }

    current_scene.__engineUpdate(deltaTime);

    physicsUpdate(); // called here for now

    current_scene.__engineLateUpdate(deltaTime);

    renderingServer.renderScene();

    std::lock_guard lock(GameObjectData::queuedelete_mut);
    //if (GameObjectData::queuedForDeletion.size() != 0) Utils::print("deletings objects : ",GameObjectData::queuedForDeletion.size());

    for (auto* obj : GameObjectData::queuedForDeletion){
        obj->parent->deleteChild(obj->getId());
    }
    GameObjectData::queuedForDeletion.clear();
}

float physDeltaTime = 0.0f;
double physLastFrame = 0.0f;

void Game::physicsUpdate(){
    // Measure speed
    // per-frame time logic
    // --------------------
    double currentFrame = glfwGetTime();
    physDeltaTime = currentFrame - physLastFrame;
    lastFrame = currentFrame;

    physics_server.step(deltaTime);

    current_scene.__enginePhysicsUpdate(physDeltaTime);
}

void Game::stop(){
    running = false;
    render_thread.join();
    glfwTerminate();
}