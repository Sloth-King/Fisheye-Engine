#include "material.h"


void Material::setUniform(const std::string name, Texture v){
    uniformTextures[name] = v;
}

void Material::setUniform(const std::string name, glm::mat4 v){
    uniformMat4s[name] = v;
} 

void Material::setUniform(const std::string name, int v){
    uniformInts[name] = v;
} 

void Material::setUniform(const std::string name, float v){
    uniformFloats[name] = v;
}

void Material::setShaderFromSource(const std::string shaderVert, std::string shaderFrag){

    
    loadShaders(shaderVert, shaderFrag);

}


void Material::bind(GLuint overrideShader) const{
    GLuint shader = overrideShader;
    if (overrideShader == 0){
        shader = _shaderPID;
        glUseProgram(_shaderPID);
    }

    int i = 0;
    // textures
    for (auto & [name, val]: uniformTextures){

        val.bind(i);

        glUniform1i(
            glGetUniformLocation(shader, name.c_str()),
            i
        );
        ++i;
    }

    // mat4
    for (auto & [name, val]: uniformMat4s){ 

        glUniformMatrix4fv(
            glGetUniformLocation(shader, name.c_str()),
            1,
            GL_FALSE,
            &val[0][0]
        );
    }

    // float
    for (auto & [name, val]: uniformFloats){ 

        glUniform1f(
            glGetUniformLocation(shader, name.c_str()),
            val
        );
    }

    // ints
    for (auto & [name, val]: uniformInts){ 

        glUniform1i(
            glGetUniformLocation(shader, name.c_str()),
            val
        );
    }
}