#version 330 core

uniform sampler2D heightmap;

uniform mat4 MVP;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 raw_uv;

out vec2 uv;
out float steepness;
out float height;

void main(){

        uv = raw_uv;
        vec4 pos = vec4(vertices_position_modelspace,1);
        pos.z += texture(heightmap, uv).x  * 0.4;

        gl_Position = MVP * pos;


        // norme carrée du gradient
        const float offset = 0.05;
        height = pos.z;
        steepness = pow(
                pow(
                        texture(heightmap, uv + vec2(offset, 0)).r - texture(heightmap, uv - vec2(offset, 0)).r,
                        2.0
                ) + 
                pow(
                        texture(heightmap, uv + vec2(0, offset)).r - texture(heightmap, uv - vec2(0, offset)).r,
                        2.0
                ),
                1.0
        ) * 20.0; // just to see it lol
}