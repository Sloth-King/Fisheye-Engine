#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <GL/glew.h>
#include "engine/external/stb_image.h"

class CubeMap {
// https://learnopengl.com/Getting-started/Textures

protected:
public:
    GLuint _texture_id = 0;

    bool __synchronized = false;

public:

    CubeMap(const std::string path_to_directory){

        loadCubemap(path_to_directory);
    }

    CubeMap(CubeMap && other)
    {
        __synchronized = other.__synchronized;
        if (__synchronized){
            _texture_id = other._texture_id;
            other.__synchronized = false; // so it doesnt free the gpu buffer

        }
    }
    CubeMap() = default;

    ~CubeMap(){
        if (__synchronized){
            glDeleteTextures(1, &_texture_id);
        }
    }

    CubeMap& operator=(CubeMap&& other){
        __synchronized = other.__synchronized;
        if (__synchronized){
            _texture_id = other._texture_id;
            other.__synchronized = false; // so it doesnt free the gpu buffer

        }

        return *this;
    }

    // pour générer des cubemaps depuis des hdris https://matheowis.github.io/HDRI-to-CubeMap/

    void loadCubemap(std::string path){ // from learnopengl

        const std::vector<std::string> filenames = {
            "/px.png",
            "/nx.png",
            "/py.png",
            "/ny.png",
            "/pz.png",
            "/nz.png"
        };

        glGenTextures(1, &_texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _texture_id);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < filenames.size(); i++)
        {
            unsigned char *data = stbi_load((path + filenames[i]).c_str(), &width, &height, &nrChannels, 3);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);

                __synchronized = true;
            }
            else
            {
                std::cout << "[CUBEMAP] couldn't load at" << (path + filenames[i]) << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        __synchronized = true;
        glFinish();
    }
    

    GLuint getTextureId(){ return _texture_id;};

    // IN CASE STRANGE THINGS HAPPEN: ALWAYS REMEMBER https://learnopengl.com/Getting-started/Textures
    void bind(GLuint slot = 0) const {
        glActiveTexture(GL_TEXTURE0+slot); // should go here actually
        glBindTexture(GL_TEXTURE_CUBE_MAP, _texture_id);

        
    }
};