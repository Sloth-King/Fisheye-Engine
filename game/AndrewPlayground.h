#pragma once

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#include "engine/includes/core.h"
#include "engine/includes/components.h"

#include "src/mapManager/mapManager.h"


using Utils::print;

void andrew(void)
{
    // Setup game
    Game game;
    game.settings.windowWidth = 720;
    game.settings.windowHeight = 720;

    game.init();
    addKeybind();

    // world
    GameObject world;
    world->addComponent<C_Transform>();

    // light

    createLightObject(world , glm::vec3(1.0f,1.0f,1.0f) , glm::vec3(1.0f));

    // scene objects

    GameObject chunk;
    chunk->addComponent<C_Transform>();
    auto *chunkMesh = chunk->addComponent<C_voxelMesh>();
    chunkMesh->create_chunk();

    // Physics
    chunk->addComponent<C_Collider>()->collider.chunk = ChunkCollider(chunkMesh);
    chunk->addComponent<C_RigidBody>()->isStatic = true;

    auto atlas = Texture(path_prefix_from_build + "resources/textures/voxelAtlas.png");
    auto roughness = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Roughness.png");
    auto metallic = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Metallic.png");
    auto normal = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Normal.png");

    Handle<MaterialPBR> material = Handle<MaterialPBR>(atlas, normal, roughness, metallic);

    chunkMesh->mesh.material = material;
    
    GameObject player = createPlayer();
    
    world->addChild(std::move(chunk));
    world->addChild(std::move(player));

    // Create scene
    Scene scene;
    scene.environment = std::move(createEnvironment("resources/textures/cloudy_skybox"));
    scene.setRoot(std::move(world));

    game.setScene(std::move(scene));

    game.start();

    game.waitGameStopped();
}