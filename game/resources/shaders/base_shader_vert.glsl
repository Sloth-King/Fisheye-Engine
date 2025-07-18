#version 330 core

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform vec3 viewVector;

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 raw_uv;

out vec2 uv;



void main(){

        uv = raw_uv;
        vec4 pos = vec4(vertices_position_modelspace,1);

        gl_Position = MVP * pos;
}