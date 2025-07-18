
#include "voxel.h"
#include <algorithm>
#include <random>
#include <array>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define SET_BIT(var,pos) (var = (var) | (1<<(pos)))

auto rng = std::default_random_engine();

void add_voxel(Mesh & mesh, glm::vec3 pos, float size, uint8_t voxelIndex, int face_mask, bool randomize_faces){

    constexpr float uv_secure_offset = 0.001;  // prevents bleeding

    if (face_mask <= 0) return;

    int v;

    float column_start =  (1.0 / (C_voxelMesh::voxelTextureSize)) * (voxelIndex -1) + uv_secure_offset;
    float column_end =  (1.0 / (C_voxelMesh::voxelTextureSize)) * (voxelIndex) - uv_secure_offset;

    float face_offset = 1.0 / 6.0;

    std::array<int, 6> faceIndices = {0, 1, 2, 3, 4, 5};

    if (randomize_faces) std::ranges::shuffle(faceIndices, rng);;

    //face front
    if (CHECK_BIT(face_mask, 0)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top right

        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[0]+1) * face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[0]+1) * face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[0] * face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[0] * face_offset) ); //front bottom right

        const glm::vec3 normal = glm::vec3(0, 0, -1);
        const glm::vec3 tangent = glm::vec3(1, 0, 0);
        const glm::vec3 bitangent = glm::vec3(0, -1, 0);

        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );


        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );
    }
    // face back
    if (CHECK_BIT(face_mask, 1)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom right
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom right

        const glm::vec3 normal = glm::vec3(0, 0, 1);
        const glm::vec3 tangent = glm::vec3(-1, 0, 0);
        const glm::vec3 bitangent = glm::vec3(0, 1, 0);


        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );
        
        mesh.triangles.push_back( Triangle(v+1, v+2, v+0) ); mesh.triangles.push_back( Triangle(v+1, v+3, v+2) );

        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[1]+1)*face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[1]+1)*face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[1]*face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[1]*face_offset) ); //front bottom right
    }
    // face right
    if (CHECK_BIT(face_mask, 2)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom right
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top right
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back top right

        const glm::vec3 normal = glm::vec3(1, 0, 0);
        const glm::vec3 tangent = glm::vec3(0, 1, 0);
        const glm::vec3 bitangent = glm::vec3(0, 0, -1);


        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );

        
        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[2]+1)*face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[2]+1)*face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[2]*face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[2]*face_offset) ); //front bottom right
    }
    // face left
    if (CHECK_BIT(face_mask, 3)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top left
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back top left

        const glm::vec3 normal = glm::vec3(-1, 0, 0);
        const glm::vec3 tangent = glm::vec3(0, -1, 0);
        const glm::vec3 bitangent = glm::vec3(0, 0, 1);

        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );


        mesh.triangles.push_back( Triangle(v+1, v+2, v+0) ); mesh.triangles.push_back( Triangle(v+1, v+3, v+2) );

        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[3]+1)*face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[3]+1)*face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[3]*face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[3]*face_offset) ); //front bottom right
    }
    // top
    if (CHECK_BIT(face_mask, 4)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front top right
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back top left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back top right

        const glm::vec3 normal = glm::vec3(0, 1, 0);
        const glm::vec3 tangent = glm::vec3(0, 0, 1);
        const glm::vec3 bitangent = glm::vec3(-1, 0, 0);

        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );


        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        
        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[4]+1)*face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[4]+1)*face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[4]*face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[4]*face_offset) ); //front bottom right

    }
    // bottom
    if (CHECK_BIT(face_mask, 5)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos - glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size/2.0f) ); //back bottom right

        const glm::vec3 normal = glm::vec3(0, -1, 0);
        const glm::vec3 tangent = glm::vec3(0, 0, -1);
        const glm::vec3 bitangent = glm::vec3(1, 0, 0);

        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent ); mesh.tangents.push_back(tangent );
        mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent ); mesh.bitangents.push_back(bitangent );

        
        mesh.triangles.push_back( Triangle(v+1, v+2, v+0) ); mesh.triangles.push_back( Triangle(v+1, v+3, v+2) );

        mesh.uvs.push_back( glm::vec2(column_start, (faceIndices[5]+1)*face_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, (faceIndices[5]+1)*face_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, faceIndices[5]*face_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, faceIndices[5]*face_offset) ); //front bottom right
    }
}



void addVoxelNonUniform(Mesh & mesh, glm::vec3 pos, glm::vec3 size, uint8_t voxelIndex, int face_mask){

    constexpr float uv_secure_offset = 0.001;  // prevents bleeding

    if (face_mask <= 0) return;

    int v;

    float column_start =  (1.0 / (C_voxelMesh::voxelTextureSize)) * (voxelIndex -1) + uv_secure_offset;
    float column_end =  (1.0 / (C_voxelMesh::voxelTextureSize)) * (voxelIndex) - uv_secure_offset;

    float face_offset = 1.0 / 6.0;

    //face front
    if (CHECK_BIT(face_mask, 0)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top right

        mesh.uvs.push_back( glm::vec2(column_start, face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, 0.0f + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, 0.0f + uv_secure_offset) ); //front bottom right

        const glm::vec3 normal = glm::vec3(0, 0, -1);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );


        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );
    }
    // face back
    if (CHECK_BIT(face_mask, 1)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom right
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom right

        const glm::vec3 normal = glm::vec3(0, 0, 1);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );

        
        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        mesh.uvs.push_back( glm::vec2(column_start, 2*face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, 2*face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, face_offset + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, face_offset + uv_secure_offset) ); //front bottom right
    }
    // face right
    if (CHECK_BIT(face_mask, 2)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom right
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top right
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back top right

        const glm::vec3 normal = glm::vec3(1, 0, 0);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );

        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        mesh.uvs.push_back( glm::vec2(column_start, 3*face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, 3*face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, 2*face_offset + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, 2*face_offset + uv_secure_offset) ); //front bottom right
    }
    // face left
    if (CHECK_BIT(face_mask, 3)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top left
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back top left

        const glm::vec3 normal = glm::vec3(-1, 0, 0);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );

        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        mesh.uvs.push_back( glm::vec2(column_start, 4*face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, 4*face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, 3*face_offset + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, 3*face_offset + uv_secure_offset) ); //front bottom right
    }
    // top
    if (CHECK_BIT(face_mask, 4)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front top right
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back top left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) + glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back top right

        const glm::vec3 normal = glm::vec3(0, 1, 0);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );

        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );
        
        mesh.uvs.push_back( glm::vec2(column_start, 5*face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, 5*face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, 4*face_offset + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, 4*face_offset + uv_secure_offset) ); //front bottom right

    }
    // bottom
    if (CHECK_BIT(face_mask, 5)){
        v = mesh.vertices.size();
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) - glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //front bottom right
        mesh.vertices.push_back( pos - glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom left
        mesh.vertices.push_back( pos + glm::vec3(size[0]/2.0f, 0.0f, 0.0f) - glm::vec3(0.0f, size[1]/2.0f, 0.0f) + glm::vec3(0.0f, 0.0f, size[2]/2.0f) ); //back bottom right

        const glm::vec3 normal = glm::vec3(0, -1, 0);
        mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal ); mesh.normals.push_back(normal );
        
        mesh.triangles.push_back( Triangle(v+1, v+0, v+2) ); mesh.triangles.push_back( Triangle(v+1, v+2, v+3) );

        mesh.uvs.push_back( glm::vec2(column_start, 6*face_offset - uv_secure_offset)); //front top left
        mesh.uvs.push_back( glm::vec2(column_end, 6*face_offset - uv_secure_offset) ); //front top right
        mesh.uvs.push_back( glm::vec2(column_start, 5*face_offset + uv_secure_offset)); //front bottom left
        mesh.uvs.push_back( glm::vec2(column_end, 5*face_offset + uv_secure_offset) ); //front bottom right
    }
}



void C_voxelMesh::voxelize(){

    glm::vec3 AABB_v1 = glm::vec3(0);
    glm::vec3 AABB_v2 = glm::vec3(container.sX, container.sY, container.sZ) * size;

    mesh.vertices.clear();
    mesh.uvs.clear();
    mesh.normals.clear();
    mesh.triangles.clear();


    //simplify
    for (int i = 0; i < container.sX; ++i){
        for (int j = 0; j < container.sY; ++j){
            for (int k = 0; k < container.sZ; ++k){
                
                int mask = 0;

                if (container.get(i, j, k) > 0){
                    
                    // si la case d'avant en x est vide
                    if (i == 0 || ( i > 0 && container.get(i-1, j, k) == 0)) SET_BIT(mask, 3);

                    // si la case d'avant en y est vide
                    if(j == 0 || ( j > 0 && container.get(i, j-1, k) == 0)) SET_BIT(mask, 5);

                    // si la case d'avant en z est vide
                    if (k== 0 || ( k > 0 && container.get(i, j, k-1)) == 0) SET_BIT(mask, 0);

                    // si la case d'après en x est vide
                    if (i == container.sX-1 || ( i < container.sX && container.get(i+1, j, k) == 0)) SET_BIT(mask, 2);

                    // si la case d'après en y est vide
                    if (j == container.sY-1 || ( j < container.sY && container.get(i, j+1, k) == 0)) SET_BIT(mask, 4);

                    // si la case d'après en z est vide
                    if (k == container.sZ-1 || ( k < container.sZ && container.get(i, j, k+1) == 0)) SET_BIT(mask, 1);

                    // create the voxel

                    glm::vec3 pos = AABB_v1 + glm::vec3(
                        size * i,
                        size * j,
                        size * k
                    );
                    add_voxel(mesh, pos + glm::vec3(size/2.0), size, container.get(i, j, k), mask, types[container.get(i, j, k)].randomizeFaces);
                }
            }
        }
    }
    //mesh.recomputeTangents();
}


void C_voxelMesh::voxelizeMarching(glm::vec3 size){
    size *= this->size;


    glm::vec3 AABB_v1 = glm::vec3(0);
    glm::vec3 AABB_v2 = glm::vec3(container.sX, container.sY, container.sZ) * size;

    mesh = Mesh();


    //simplify
    for (int i = 0; i < container.sX; ++i){
        for (int j = 0; j < container.sY; ++j){
            for (int k = 0; k < container.sZ; ++k){
                

                int mask = 0;
                if (container.get(i, j, k) > 0){

                                        
                    // si la case d'avant en x est vide
                    if (i == 0 || ( i > 0 && container.get(i-1, j, k) == 0)) SET_BIT(mask, 3);

                    // si la case d'avant en y est vide
                    if(j == 0 || ( j > 0 && container.get(i, j-1, k) == 0)) SET_BIT(mask, 5);

                    // si la case d'avant en z est vide
                    if (k== 0 || ( k > 0 && container.get(i, j, k-1)) == 0) SET_BIT(mask, 0);

                    // si la case d'après en x est vide
                    if (i == container.sX-1 || ( i < container.sX && container.get(i+1, j, k) == 0)) SET_BIT(mask, 2);

                    // si la case d'après en y est vide
                    if (j == container.sY-1 || ( j < container.sY && container.get(i, j+1, k) == 0)) SET_BIT(mask, 4);

                    // si la case d'après en z est vide
                    if (k == container.sZ-1 || ( k < container.sZ && container.get(i, j, k+1) == 0)) SET_BIT(mask, 1);

                    // create the voxel

                    glm::vec3 pos = AABB_v1 + glm::vec3(
                        size[0] * i,
                        size[1] * j,
                        size[2] * k
                    );
                    addVoxelNonUniform(mesh, pos + size/2.0f, size, container.get(i, j, k), mask);
                }
            }
        }
    }
}
