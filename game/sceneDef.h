#pragma once

#pragma once

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/includes/core.h"
#include "engine/includes/components.h"

#include "src/mapManager/mapManager.h"
#include "src/controller/playerController.h"



using Utils::print;

static std::string path_prefix_from_build = "../game/";


void addKeybind(){
    Input::addKeybind(
        "move_right",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_D
        )
    );
    Input::addKeybind(
        "move_left",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_A
        )
    );

    Input::addKeybind(
        "move_forward",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_W
        )
    );
    Input::addKeybind(
        "move_backwards",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_S
        )
    );

    Input::addKeybind(
        "move_down",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_LEFT_SHIFT
        )
    );
    Input::addKeybind(
        "move_up",
        KeySpec(
            BINDTYPE_KEYBOARD,
            GLFW_KEY_SPACE
        )
    );

}

GameObject createPlayer(){

    const glm::vec3 cameraOffset(1.3, 0.0, -3.75);

    GameObject player;

    Mesh submarine = ResourceLoader::load_mesh_obj(path_prefix_from_build + "resources/meshes/sousmarin_v2.obj");
    //Mesh submarine = ResourceLoader::load_mesh_obj(path_prefix_from_build + "resources/meshes/unit_sphere.obj");

    submarine.material = Handle<MaterialPBR>(
        Texture(path_prefix_from_build + "resources/textures/submarine.jpg")
    );

    auto* playerTransform = player->addComponent<C_Transform>();
    playerTransform->move(glm::vec3(0.05, 0.3, 0.05));
    playerTransform->setScale(glm::vec3(0.07, 0.07, 0.07));
    

    player->addComponent<C_Camera>() -> offset = cameraOffset;

    player->addComponent<C_Mesh>()->mesh = submarine;

    player->addComponent<C_PlayerController>();
    
    player->addComponent<C_Collider>()->collider.sphere = SphereCollider(glm::vec3(0.0) , 1);
    auto *playerBody = player->addComponent<C_RigidBody>();
    playerBody->gravityScale = 0.0f;
    playerBody->restitution = 0.0f;
    playerBody->damping = 0.5f;
    
    
    return player;
}


Environment createEnvironment(const std::string & skybox_path = "resources/textures/skybox"){
    Environment env;

    auto cubemap = CubeMap(
        path_prefix_from_build + skybox_path.c_str()
    );

    env.skybox = std::move(Skybox( std::move(cubemap)));

    return env;
}

GameObject createLightObject(GameObject & parent, glm::vec3 position, glm::vec3 color){

    GameObject light;

    auto* lightComponent = light->addComponent<C_Light>();

    lightComponent->light.color = color;
    lightComponent->light.intensity = 1.0;
    light->addComponent<C_Transform>()->setPosition(position);
    
    parent->addChild(std::move(light));

    return light;
}

Scene createScene(){
    Scene scene;

    scene.environment = std::move(
        createEnvironment()
    );

    // world
    GameObject world;
    world->addComponent<C_Transform>();

    auto* v = world->addComponent<C_MapManager>();

    GameObject player = createPlayer();

    world->addChild(std::move(player));

    v->player = player;
    v->initChunks();




    
    createLightObject(
        world,
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0, 1.0, 0.0)
    );
    


    scene.setRoot(std::move(world));

    return scene;
}



void setupScene(Game& game){
    Scene scene;
    GameObject world;


    Mesh cube = ResourceLoader::load_mesh_obj("../game/resources/meshes/supercube.obj");

    // Les handles sont des objets référence à des ressources. Tant qu'au moins une handle existe, la ressource
    // n'est pas libérée.
    cube.material = Handle<MaterialPBR>(Texture("../game/resources/textures/logo.jpg"));



    C_Transform* t = world->addComponent<C_Transform>(); 
    t->setPosition(glm::vec3(0, -0.2, 0));
    t->setScale(glm::vec3(0.1, 0.1, 0.1));

    world->addComponent<C_Mesh>() -> mesh = cube; 





    // light

    GameObject light;

    auto* lightComponent = light->addComponent<C_Light>();

    lightComponent->light.color = glm::vec3(1.0, 1.0, 1.0);
    lightComponent->light.intensity = 100.0;
    light->addComponent<C_Transform>()->setPosition(glm::vec3(10.0, 10.0, 5.0));
    
    world->addChild(std::move(light));


    Environment env;

    auto cubemap = CubeMap(
        path_prefix_from_build + "resources/textures/skybox"
    );

    env.skybox = Skybox( std::move(cubemap));
    scene.environment = std::move(env);

    scene.setRoot(std::move(world));
  
    game.setScene(std::move(scene)); // on donne l'ownership de la scène au jeu
}




void game( void )
{

    Game game;
    game.settings.windowWidth = 1600;
    game.settings.windowHeight = 900;

    game.init();

    addKeybind();
    
    game.setScene(
        createScene()
    );
    

    //setupScene(game);
    game.start();


    game.waitGameStopped();
}