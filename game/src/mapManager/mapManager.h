#pragma once 
#include "engine/includes/core.h"
#include "engine/includes/components.h"
#include <vector>
#include <functional>
#include <string>
#include "game/src/voxel/voxel.h"
#include <thread>
#include <unistd.h>

unsigned long long getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}




/*
three referentials:
- ChunkIdx is the indices of the chunks in the pseudo 3D chunks array
- ChunkCoords is the coordinate in the global chunk grid. Can differ from the first one if the player is not at the origin
- block_position is the position in the global world but as an integer. Helps for the offset in the mapgeneration file.
*/


class C_MapManager: public Component{
    GLuint shaderPID;
    Texture atlas;
    Texture roughness;
    Texture metallic;
    Texture normal;

    Handle<MaterialPBR> material;

public:
    
    const int chunkRadiusXZ = 7; // 7
    const int chunkRadiusY = 2; // 2

    const float world_scale = 0.05f;

    std::vector < GameObject > chunks;

    GameObject player;

    glm::ivec3 current_player_pos_in_world;

    glm::ivec3 current_player_chunk_in_map;

    C_MapManager(){
        chunks.resize((2*chunkRadiusXZ+1) * (2*chunkRadiusY+1) * (2*chunkRadiusXZ+1));

        std::string path_prefix_from_build = "../game/";
        std::string vertex_shader_filename = path_prefix_from_build + "resources/shaders/voxel_shader_vert.glsl";
        std::string fragment_shader_filename = path_prefix_from_build + "resources/shaders/voxel_shader_frag.glsl";

        shaderPID = loadShadersFromFileGLSL(vertex_shader_filename.c_str(), fragment_shader_filename.c_str());

        atlas = Texture(path_prefix_from_build + "resources/textures/voxelAtlas.png");
        roughness = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Roughness.png");
        metallic = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Metallic.png");
        normal = Texture(path_prefix_from_build + "resources/textures/voxelAtlas_Normal.png");

        material = Handle<MaterialPBR>(atlas, normal, roughness, metallic);
        
    }

    inline size_t convertCoord(const glm::ivec3 & v){
        return (v.z * (2 * chunkRadiusXZ + 1) * (2 * chunkRadiusY + 1)) + (v.y * (2 * chunkRadiusXZ + 1)) + v.x;
    }

    GameObject getChunkAt(glm::ivec3 v){ // ALWAYS relative to player. (0, 0) is the player, this function does the fun conversion part
        v = {
            Utils::posmod(v.x + current_player_chunk_in_map.x, 2 * chunkRadiusXZ + 1),
            Utils::posmod(v.y + current_player_chunk_in_map.y, 2 * chunkRadiusY + 1),
            Utils::posmod(v.z + current_player_chunk_in_map.z, 2 * chunkRadiusXZ + 1)
        };

        return chunks.at(convertCoord(v));
    }

    void setChunkAt(glm::ivec3 v, GameObject chunk){ // ALWAYS relative to player. (0, 0) is the player, this function does the fun conversion part
        v = {
            Utils::posmod(v.x + current_player_chunk_in_map.x, 2 * chunkRadiusXZ + 1),
            Utils::posmod(v.y + current_player_chunk_in_map.y, 2 * chunkRadiusY + 1),
            Utils::posmod(v.z + current_player_chunk_in_map.z, 2 * chunkRadiusXZ + 1)
        };

        //chunks.at((v.z * chunkRadiusXZ * chunkRadiusY) + (v.y * chunkRadiusXZ) + v.x) = chunk;
        chunks.at(convertCoord(v)) = chunk;
    }

    glm::ivec3 getPlayerChunkCoords() const {
        if (!player) return glm::ivec3(0);

        auto pos = player->getComponent<C_Transform>()->getGlobalPosition();

        pos /= world_scale;

        pos.x /= CHUNK_SIZE_XZ;
        pos.y /= CHUNK_SIZE_Y;
        pos.z /= CHUNK_SIZE_XZ;

        return glm::round(pos);
    }

    inline GameObject createChunk(const glm::ivec3 & chunkCoord){

        GameObject g;

        const glm::vec3 scale(CHUNK_SIZE_XZ, CHUNK_SIZE_Y, CHUNK_SIZE_XZ);
        
        glm::vec3 global_pos = glm::vec3(chunkCoord) * scale;

        auto* c_transform = g->addComponent<C_Transform>();
        c_transform->setPosition((global_pos - 0.5f * scale) * world_scale); // - 0.5 to be centered around player

        auto* c_voxelmesh = g->addComponent<C_voxelMesh>();
        c_voxelmesh->size = world_scale;

        c_voxelmesh->mesh.material = material;

        c_voxelmesh->create_chunk(glm::round(global_pos));

        //g->addComponent<C_Collider>()->collider.chunk = ChunkCollider(c_voxelmesh);
        //g->addComponent<C_RigidBody>()->isStatic = true;        

        getOwner()->addChild(std::move(g));
        return g;
    }
    
    inline glm::ivec3 chunkIdxToChunkCoord(glm::ivec3 chunkIdx){
        return getPlayerChunkCoords() + chunkIdx;
    }

    
    
    void initChunks(){

        Utils::time<'c'>(); // init (print pas la première fois)

        current_player_pos_in_world = getPlayerChunkCoords();

        for (int i = -chunkRadiusXZ; i <= chunkRadiusXZ; ++i){
            for (int j = chunkRadiusY; j >= -chunkRadiusY; --j){ //top down rendering for better depth discarding
                for (int k = -chunkRadiusXZ; k <= chunkRadiusXZ; ++k){
                    auto chunkIdx = glm::ivec3(i, j, k);
                    GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                    setChunkAt(
                        chunkIdx,
                        chunk
                    );
                }
            }
        }
        Utils::time<'c'>();
    }


    virtual void _onUpdate(float deltaTime) override {

        static std::thread thread;

        auto new_player_pos_in_world = getPlayerChunkCoords();

        if (new_player_pos_in_world != current_player_pos_in_world){
            if (thread.joinable()) thread.join(); // end execution of last thread. Idk if it's clean

            thread = std::thread(&C_MapManager::updateChunks, this, current_player_pos_in_world, new_player_pos_in_world);
            current_player_pos_in_world = new_player_pos_in_world;
        }
    }



    void updateChunks(glm::ivec3 current_player_pos_in_world, glm::ivec3 new_player_pos_in_world) {
        //return; //TODO finir ça
        Utils::time<'a'>(true);
        auto delta = new_player_pos_in_world - current_player_pos_in_world;



        // remove and replace all chunks
        int sXZ =  2 * chunkRadiusXZ + 1;
        int sY = 2 * chunkRadiusY + 1;

        current_player_chunk_in_map += delta;

        current_player_chunk_in_map.x = Utils::posmod(current_player_chunk_in_map.x, sXZ);
        current_player_chunk_in_map.y = Utils::posmod(current_player_chunk_in_map.y, sY);
        current_player_chunk_in_map.z = Utils::posmod(current_player_chunk_in_map.z, sXZ);

        int number_on_x = std::min(abs(delta.x), sXZ);

        // a lot of loops. Feels scary but they're all small on at least one axis
        const glm::ivec3 radius_vec(chunkRadiusXZ, chunkRadiusY, chunkRadiusXZ);
        // on x
        if (delta.x > 0)
            for (int i = 0; i < number_on_x; ++i){
                for (int j = sY-1; j >= 0; --j){
                    for (int k = 0; k < sXZ; ++k){
                        auto chunkIdx = -glm::ivec3(i, j, k)  + radius_vec;

                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }
        else if (delta.x < 0) // do nothing on 0
            for (int i = 0; i < number_on_x; ++i){
                for (int j = sY-1; j >= 0; --j){
                    for (int k = 0; k < sXZ; ++k){
                        auto chunkIdx = glm::ivec3(i,j,k) - radius_vec;

                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }

        
        // on y
        int number_on_y = std::min(abs(delta.y), sY);
        if (delta.y > 0)
            for (int i = number_on_x; i < sXZ; ++i){
                for (int j = number_on_y-1; j >= 0; --j){ // we don't have to do the corner we've already done
                    for (int k = 0; k < sXZ; ++k){
                        auto chunkIdx = -glm::ivec3(i, j, k) + radius_vec;

                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }
        else if (delta.y < 0)
            for (int i = number_on_x; i < sXZ; ++i){
                for (int j = number_on_y-1; j >= 0; --j){
                    for (int k = 0; k < sXZ; ++k){
                        auto chunkIdx = glm::ivec3(i, j, k) - radius_vec;

                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }



        // on z
        int number_on_z = std::min(abs(delta.z), sXZ);
        if (delta.z > 0)
            for (int i = number_on_x; i < sXZ; ++i){
                for (int j = sY-1; j >= number_on_y; --j){ // we don't have to do the corner we've already done
                    for (int k = 0; k < number_on_z; ++k){
                        auto chunkIdx = -glm::ivec3(i, j, k)  + radius_vec;

                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }
        else if (delta.z < 0)
            for (int i = number_on_x; i < sXZ; ++i){
                for (int j = sY-1; j >= number_on_y; --j){
                    for (int k = 0; k < number_on_z; ++k){
                        auto chunkIdx = glm::ivec3(i, j, k) - radius_vec;
                        getChunkAt(chunkIdx)->queueDelete();
                        GameObject chunk = createChunk(chunkIdxToChunkCoord(chunkIdx));
                        setChunkAt(
                            chunkIdx,
                            chunk
                        );
                    }
                }
            }

        Utils::time<'a'>("un mouvement");
    }
};