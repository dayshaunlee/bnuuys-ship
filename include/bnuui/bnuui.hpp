#pragma once

#include <OpenGL/gltypes.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <vector>
#include "GLFW/glfw3.h"
#include "tinyECS/components.hpp"

namespace bnuui {

// Base class for different types of UI Elements.
// All UI Elements will have a scale and position.
class Element {
public:
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;
    
    glm::vec3 color;

    TEXTURE_ASSET_ID texture;
    EFFECT_ASSET_ID effect;
    GEOMETRY_BUFFER_ID geometry;

    bool visible = true;

    std::vector<std::shared_ptr<Element>> children;
    Element* parent = nullptr;
};



// This is just a wrapper for a list of UI Elements.
// This is also the Public UI that will be used in our scenes.
class SceneUI {
public:
    bool insert(Element elem);
    bool remove(Element elem);
private: 
    std::vector<Element> ui_elems;
};

}
