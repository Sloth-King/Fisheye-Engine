#version 460 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 raw_uv;
layout(location = 2) in vec3 raw_normals;
layout(location = 3) in vec3 raw_aTangent;
layout(location = 4) in vec3 raw_aBitangent;

uniform mat4 Model;
uniform mat4 MVP;

out vec2 uv;
out vec3 normal;
out mat3 TBN;

void main(){

        uv = raw_uv;
        mat4 norm_transform = Model;

        normal = normalize((norm_transform * vec4(raw_normals, 0)).xyz);
        vec3 tangent = normalize((norm_transform * vec4(raw_aBitangent, 0)).xyz);
        vec3 biTangent = normalize((norm_transform * vec4(raw_aBitangent, 0)).xyz);
        
        vec4 pos = vec4(vertices_position_modelspace,1);

        gl_Position = MVP * pos;

        //TBN matrix
        TBN = mat3(tangent, biTangent, normal);
}