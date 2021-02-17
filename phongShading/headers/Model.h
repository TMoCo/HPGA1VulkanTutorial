//
// A model class for handling model related operations and data
//

#ifndef MODEL_H
#define MODEL_H

#include "Vertex.h" // vertex struct

#include <string> // string for model path
#include <vector> // vector container

//#include <vulkan/vulkan_core.h>

class Model {
public:
	void loadModelle(const std::string& path);

    //
    // object data
    //

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

};

#endif // !MODEL_H