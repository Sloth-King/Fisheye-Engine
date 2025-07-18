#include "voxel.h"
#include "im3d.h"
#include "engine/external/FastNoiseLite.h"
#include <random>
bool setup = false;
const int seed = 15987; // here for now
FastNoiseLite baseTerrainNoise(seed);
FastNoiseLite sharpTerrainNoise(seed);
FastNoiseLite rugosityNoise(seed);

FastNoiseLite rockNoise(seed);
Mesh algaeMesh;

void setupNoise(){
    baseTerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    baseTerrainNoise.SetFrequency(0.02);

    sharpTerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    sharpTerrainNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    sharpTerrainNoise.SetFrequency(0.1);

    rockNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    rockNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    rockNoise.SetFrequency(0.03);

    rugosityNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    rugosityNoise.SetFrequency(0.01);

    setup = true; // otherwise memory overflow from all the textures. Idk why they wouldnt be destroyed at some point but ...


    // setup meshes aswell

    algaeMesh = ResourceLoader::load_mesh_obj("../game/resources/meshes/algae.obj");
    algaeMesh.material = Handle<MaterialPBR>(Texture("../game/resources/textures/submarine.jpg"));
}


std::random_device r;

// Choose a random mean between 1 and 6
std::default_random_engine e1(r());
std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

inline float randval(){
    return uniform_dist(e1);
}



void spawnAlgae(const glm::ivec3 & local_position, C_voxelMesh & chunk){

    GameObject algae;

    float size = chunk.size;

    auto*  transform = algae->addComponent<C_Transform>();
    transform->setPosition((glm::vec3(local_position ) + glm::vec3(0.0, 1.0, 0.0))* size);
    transform->setScale(glm::vec3(size));
    transform->rotate(glm::vec3(0, randval() * 2 * M_PI * 10, 0.0));

    algae->addComponent<C_Mesh>()->mesh = algaeMesh;
    auto* light = algae->addComponent<C_Light>();
    light->light.color = glm::vec3(1.0, 0.02, 1.0);
    light ->light.intensity = 1.0;

    chunk.getOwner()->addChild(std::move(algae));
}
int nbalgae = 0;
uint8_t generateVoxel(const glm::ivec3 & global_position, const glm::ivec3 & local_position, C_voxelMesh & chunk){

    // if (global_position == glm::ivec3(1.0, 1.0, 1.0)) return 1;
    // return 0;

    const int fac = 5;

    float rugosity = rugosityNoise.GetNoise((float)global_position.x, (float)global_position.z);

    float h = baseTerrainNoise.GetNoise((float)global_position.x, (float)global_position.z) * fac;
    
    float rock = baseTerrainNoise.GetNoise((float)global_position.x, (float)global_position.z);

    h += sharpTerrainNoise.GetNoise((float)global_position.x, (float)global_position.z) * fac * 10 * pow(rugosity, 3);
    
    VOXEL_IDX_TYPE result = 0;
    bool spawn_algae = false;
    // base terrain

    if (global_position.y < -50){
        result = 0;
    }

    if (rugosity < -0.65){

        if (global_position.y < h - rugosity * (-h / 3) - 1){
            result = 2;
        }

        if (global_position.y <  h - rugosity * 3 - 1){
            result = 3;
        }

        if (global_position.y < h - rock * 5 - 4){
            result = 2;
        }

        if (global_position.y < h - rock * rugosity * 7 - 20){
            result = 7;
        }


    } else if (rugosity < 0.65){

        if (global_position.y < h - rock * 10 - 13){
            result = 7;
        }
        else if (global_position.y < h - rugosity * 3 - 6){
            result = 2;
        }

        else if (global_position.y <  h - rock * 3 - 3){
            result = 3;
        }
        else if (global_position.y < h-1){
            result = 4;
        }
        else if (global_position.y <= h){
            result = 4;
            spawn_algae = true;
        }


    } else {

        if (global_position.y < h - rugosity * (-h / 3) - 1){
            result = 2;
        }

        if (global_position.y <  h - rugosity * (-h / 2) - 1){
            result = 3;
        }

        if (global_position.y < h - rugosity * 3 - 7){
            result = 2;
        }

        if (global_position.y < h - rock * 10 - 13){
            result = 7;
        }

    }

    // rocks

    if (pow(rock, 7) > 0.15 && global_position.y > h- abs(rugosity) * 3 && global_position.y  < h + abs(rock) *2 ){
        result = 7;
    }

    //result = 6;
    if (spawn_algae && randval() < 0.002){
        spawnAlgae(local_position, chunk);
        // Utils::print(nbalgae++);
    }


    return result;
}

void generateStrate(C_voxelMesh & chunk, int k, const glm::ivec3 & offset){

    glm::vec3 global_position;
    glm::ivec3 local_position;

    for (int i = 0; i < CHUNK_SIZE_XZ; ++i){
        for (int j = 0; j < CHUNK_SIZE_XZ; ++j){

            local_position = glm::ivec3(i, k, j);

            global_position = local_position + offset;

            chunk.container.set(
                i, k, j,
                generateVoxel(global_position, local_position, chunk)
                );
        }
    }
}

void generateChunk(C_voxelMesh& chunk, const glm::ivec3 offset){
    if (!setup)
        setupNoise();

    chunk.container = VoxelContainer(CHUNK_SIZE_XZ, CHUNK_SIZE_Y, CHUNK_SIZE_XZ);

    for (int k = 0; k < CHUNK_SIZE_Y; ++k){
        
        generateStrate(chunk, k, offset);
    }
}



void generateFrom3DTexture(VoxelContainer & container, std::string path, int dimX, int dimY, int dimZ, int threshold, int trueValue){
    

    Image im = read_img_file(path.c_str() , dimX , dimY , dimZ);

    saveHistogramData(im, path + "HISTO.dat", dimX, dimY, dimZ);

    container = VoxelContainer(dimX, dimY, dimZ);



    for(int i = 0 ; i < dimX * dimY * dimZ ; i++){
        auto v = (unsigned int)im.data[i];
        
        //on met à trueValue les valeurs au dessus du seuil et à 0 les autres
        container.data[i] = (v >= threshold)? trueValue : 0;
    }
}