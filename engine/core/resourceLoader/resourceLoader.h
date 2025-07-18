#pragma once

#include "engine/render/mesh/mesh.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

namespace ResourceLoader{



Mesh load_mesh_off(std::string filename);


Mesh load_mesh_obj(std::string path);
}

