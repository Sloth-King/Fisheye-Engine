#include "mesh.h"

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

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include "engine/core/resourceLoader/resourceLoader.h"
#include "engine/utils/utils.h"



//// generates a base plane
Mesh Mesh::gen_tesselatedSquare(int nX, int nY, float sX, float sY){
    Mesh o_mesh;

    for (int u = 0; u<nX; ++u){
        for (int v = 0; v<nY; ++v){

            float px = 1. / float(nX) * u;
            float py = 1. / float(nY) * v;

            o_mesh.vertices.push_back(
                glm::vec3(
                    (px -0.5) * sX,
                    (py -0.5) * sY,
                    0
                )
            );
            o_mesh.uvs.push_back(
                glm::vec2(
                    px,
                    py
                )
            );

        }
    }
    for (int i = 0; i<nY-1; ++i){
        for (int j = 0; j<nX-1; ++j){

            o_mesh.triangles.push_back(
                Triangle(
                    nX * j + i,
                    nX*(j+1) +i,
                    nX*(j+1) +i+1
                )
            );
            o_mesh.triangles.push_back(
                Triangle(
                    nX * j + i,
                    nX*(j+1) +i+1,
                    nX*(j) +i+1
                )
            );
            
            
        }
    }
    o_mesh.recomputeNormals();
    return o_mesh;
}