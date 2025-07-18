
#include "engine/render/mesh/mesh.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <cstring>

#include <string>
#include <unordered_map>

namespace ResourceLoader{



Mesh load_mesh_off(std::string filename) {
    Mesh res;

    std::ifstream in (filename.c_str ());
    if (!in){
        std::cout << "ERROR [.OFF LOADER]: can't find or open the file (" << filename << ")" << std::endl;
        return res;
    }
    std::string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    res.vertices.resize (sizeV);
    res.triangles.resize (sizeT);
    
    for (unsigned int i = 0; i < sizeV; i++){
        in >> res.vertices[i][0];
        in >> res.vertices[i][1];
        in >> res.vertices[i][2];
    }


    int s;
    for (unsigned int i = 0; i < sizeT; i++) {
        in >> s;
        for (unsigned int j = 0; j < 3; j++)
            in >> res.triangles[i].v[j];
    }
    in.close();
    res.recomputeNormals();
    return res;
}

// for the uonrdered map in obj loader
struct KeyFuncs
{
    size_t operator()(const glm::ivec3& k)const
    {
        return std::hash<int>()(k.x) ^ std::hash<int>()(k.y) ^ std::hash<int>()(k.z);
    }

    bool operator()(const glm::ivec3& a, const glm::ivec3& b)const
    {
            return a == b;
    }
};


Mesh load_mesh_obj(std::string path){

    std::vector<TRI_IDX_TYPE> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    Mesh mesh;

    FILE * file = fopen(path.c_str(), "r");
    if( file == NULL ){
        std::cout << "ERROR [.OBJ LOADER]: can't find or open the file (" << path << ")" << std::endl;
        return mesh;
    }

    while( 1 ){

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader

        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                std::cout << "ERROR [.OBJ LOADER]: file formatting not recognized. Please try to convert or re-export it." << std::endl;
                return mesh;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }else{
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    std::unordered_map< glm::ivec3, size_t, KeyFuncs, KeyFuncs> already_seen_triplets;


    // For each vertex of each triangle

    glm::ivec3 final_tri_idx;

    for( unsigned int i=0; i<vertexIndices.size(); i+=3 ){

        for (size_t j = 0; j < 3; ++j){
            glm::ivec3 indices(vertexIndices[i+j], uvIndices[i+j], normalIndices[i+j]);

            if (already_seen_triplets.contains(indices)){
                final_tri_idx[j] = already_seen_triplets[indices];

            } else { // on ajoute le point
                final_tri_idx[j] = mesh.vertices.size();
                already_seen_triplets[indices] = mesh.vertices.size();
                mesh.vertices.push_back(temp_vertices[indices[0]-1]);
                mesh.uvs.push_back(temp_uvs[indices[1]-1]);
                mesh.normals.push_back(temp_normals[indices[2]-1]);

            }

        }
        mesh.triangles.push_back(Triangle(final_tri_idx[0], final_tri_idx[1], final_tri_idx[2]));
    }

    fclose(file);

    mesh.recomputeTangents();

    return mesh;
}

}
