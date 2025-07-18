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

#include "engine/external/common/tangentspace.hpp"


GLuint TRI_GL_TYPE = GL_UNSIGNED_INT; // change with TRI_IDX_TYPE in mesh.h!


void Mesh::synchronize() const {

    if (_synchronized){ // maybe we just want to resync
        unsynchronize();
    }
    
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    glGenBuffers(1, &_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);


    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well

    glGenBuffers(1, &_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle), &triangles[0] , GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, _VBO);


    glGenBuffers(1, &_UV);
    glBindBuffer(GL_ARRAY_BUFFER, _UV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0] , GL_STATIC_DRAW);

    glGenBuffers(1, &_NORMALS);
    glBindBuffer(GL_ARRAY_BUFFER, _NORMALS);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0] , GL_STATIC_DRAW);

    glGenBuffers(1, &_TANGENT);
    glBindBuffer(GL_ARRAY_BUFFER, _TANGENT);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0] , GL_STATIC_DRAW);

    glGenBuffers(1, &_BITANGENT);
    glBindBuffer(GL_ARRAY_BUFFER, _BITANGENT);
    glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0] , GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);

    // Positions attribute buffer

    glVertexAttribPointer(
        0,                  // attribute
        3,    // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,  // stride
        (void*)0            // array buffer offset
    );


    // Uvs attribute buffer
    glBindBuffer(GL_ARRAY_BUFFER, _UV);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(1);


    // normals
    glBindBuffer(GL_ARRAY_BUFFER, _NORMALS);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(2);

    // tangents
    glBindBuffer(GL_ARRAY_BUFFER, _TANGENT);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(3);


    // bitangents
    glBindBuffer(GL_ARRAY_BUFFER, _BITANGENT);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(4);



    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

    glBindVertexArray(0);

    // textures

    // TODO synchronize texture here aswell but not now since texture still can't "unsychronize"

    /*
    glUseProgram(shaderPID);
    
    for (auto t: textures){
        glUniform1i(
            glGetUniformLocation(shaderPID, t.second.c_str()),
            t.first.getTextureId()
        );
    }*/

    _synchronized = true;
}

void Mesh::renderForward(const glm::mat4 & vpMatrix, glm::vec3 fv, const glm::mat4 & outside_transform) const{

    if (vertices.empty()) return;
    if (!_synchronized){ // FIXME branch prediction may bottleneck a little here? idk
        synchronize();
    }

    glBindVertexArray(_VAO);
    glUseProgram(shaderPID);


    int i = 0;
    for (auto & t: textures){

        t.first.bind(i);



        glUniform1i(
            glGetUniformLocation(shaderPID, t.second.c_str()),
            i
        );
        ++i;
    }
    
    // todo just tranfer a struct to the gpu with all interesting data
    GLuint mvpUniformLocation = glGetUniformLocation(shaderPID, "MVP");
    GLuint camForardLocation = glGetUniformLocation(shaderPID, "view");
    GLuint modelLocation = glGetUniformLocation(shaderPID, "Model");

    glm::mat4 MVP = vpMatrix * outside_transform * transform; // transform the VP into MVP



    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, &MVP[0][0]);

    glUniform3fv(camForardLocation, 1, &fv[0]);

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE,  &transform[0][0]);



    
    glDrawElements(
                GL_TRIANGLES,      // mode
                triangles.size()*3,    // count
                TRI_GL_TYPE,   // type
                (void*)0           // element array buffer offset
                );
    
    glUseProgram(0);
    glBindVertexArray(0);
}


void Mesh::renderDeferred(const glm::mat4 & vpMatrix, glm::vec3 fv, const glm::mat4 & outside_transform, GLuint gShader) const{

    if (vertices.empty()) return;
    if (!_synchronized){ // FIXME branch prediction may bottleneck a little here? idk
        synchronize();
    }

    glBindVertexArray(_VAO);

    glm::mat4 MODEL = outside_transform * transform;

    glm::mat4 MVP = vpMatrix * MODEL; // transform the VP into MVP

        // todo just tranfer a struct to the gpu with all interesting data
    GLuint mvpUniformLocation = glGetUniformLocation(gShader, "MVP");
    GLuint modelUniformLocation = glGetUniformLocation(gShader, "MODEL");

    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, &MODEL[0][0]);

    material->bind(gShader);
    
    
    glDrawElements(
                GL_TRIANGLES,      // mode
                triangles.size()*3,    // count
                TRI_GL_TYPE,   // type
                (void*)0           // element array buffer offset
                );
    
    glBindVertexArray(0);
}

void Mesh::unsynchronize() const {
    glDeleteBuffers(1, &_VBO);
    glDeleteBuffers(1, &_EBO);
    glDeleteBuffers(1, &_UV);
    glDeleteBuffers(1, &_NORMALS);
    glDeleteProgram(shaderPID);
    glDeleteVertexArrays(1, &_VAO);
    
    _synchronized = false;

}

void Mesh::setShader(std::string vertex_shader, std::string fragment_shader){
    shaderPID = loadShadersFromFileGLSL(vertex_shader.c_str(), fragment_shader.c_str());
}

void Mesh::setShaderPid(GLuint pid){
    shaderPID = pid;
}

void Mesh::recomputeNormals () {
    normals.resize(vertices.size());
    for (unsigned int i = 0; i < vertices.size (); i++)
        normals[i] = glm::vec3(0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < triangles.size (); i++) {
        glm::vec3 e01 = vertices[triangles[i].v[1]] -  vertices[triangles[i].v[0]];
        glm::vec3 e02 = vertices[triangles[i].v[2]] -  vertices[triangles[i].v[0]];
        glm::vec3 n = glm::cross(e01, e02);
        n = glm::normalize(n);
        for (unsigned int j = 0; j < 3; j++)
            normals[triangles[i].v[j]] += n;
    }
    for (unsigned int i = 0; i < vertices.size (); i++)
        normals[i] = glm::normalize(normals[i]);

    recomputeTangents();
}

void Mesh::recomputeTangents(){
    computeTangentBasis(vertices, uvs, normals, tangents, bitangents);
}

