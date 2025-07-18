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



using Utils::print;



GameObject createPlayer(){
    GameObject player;

    Mesh submarine = ResourceLoader::load_mesh_obj(path_prefix_from_build + "resources/meshes/sousmarin_v1.obj");
    submarine.addTexture( Texture(path_prefix_from_build + "resources/textures/submarine.jpg"), "albedo" );

    auto* playerTransform = player->addComponent<C_Transform>();
    playerTransform->setScale(glm::vec3(0.07, 0.07, 0.07));
    playerTransform->move(glm::vec3(0, 1, 0));


    player->addComponent<C_Mesh>()->mesh = submarine;
    
    return player;
}

Environment createEnvironment(const std::string & skybox_path = "resources/textures/skybox"){
    Environment env;

    auto cubemap = CubeMap(
            path_prefix_from_build + skybox_path.c_str()
        );

    env.skybox = Skybox( std::move(cubemap));

    return env;
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
    v->initChunks();

    GameObject player = createPlayer();

    //world.addChild(std::move(player));


    // tests transforms
    
    Mesh submarine = Mesh::gen_tesselatedSquare(1.0, 1.0);

    GameObject transformtest1;
    auto* transformcomp1 = transformtest1->addComponent<C_Transform>();
    transformcomp1 -> setScale(glm::vec3(0.5, 0.5, 0.5));
    transformcomp1 -> setRotation(glm::vec3(M_PI, 0.0, 0.0));
    transformcomp1 -> move(glm::vec3(0.0, 0.5, 0.0));
    
    
    GameObject transformtest2;
    transformtest2->addComponent<C_Transform>();
    
    

    transformtest1->addChild(std::move(transformtest2));
    world->addChild(std::move(transformtest1));

    scene.setRoot(std::move(world));

    transformtest2->getComponent<C_Transform>()->moveGlobal(glm::vec3(0.0, 1.0, 0.0));
    transformtest2->getComponent<C_Transform>()->rotateGlobal(glm::vec3(-M_PI, 0, 0.0));

    transformtest2->getComponent<C_Transform>()->printGlobal();
    //t2->printGlobal();

    return scene;
}

void tom( void )
{

    Game game;
    game.settings.windowWidth = 720;
    game.settings.windowHeight = 720;

    game.init();
    
    game.setScene(
        createScene()
    );

    game.start();

    game.waitGameStopped();
}