#pragma once

#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <utility>
#include <vector>
#include "tinyECS/components.hpp"

namespace bnuui {

// Base class for different types of UI Elements.
// All UI Elements will have a scale and position.
class Element {
public:
    // Appearance of the Element
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

    glm::vec2 offset;   // This determines the bounding box and is hard coded for each texture.
    
    glm::vec3 color;

    TEXTURE_ASSET_ID texture;
    EFFECT_ASSET_ID effect;
    GEOMETRY_BUFFER_ID geometry;

    bool visible = true;
    
    // Parent/Child semantics for the Element
    std::vector<std::shared_ptr<Element>> children;
    std::weak_ptr<Element> parent;

    // Interactivity
    bool hovering = false;
    bool active = false;

    void setOnActive(std::function<void(Element&)> callback) {
        onActive = std::move(callback);
    }

    void setOnClick(std::function<void(Element&)> callback) {
        onClick = std::move(callback);
    }

    void setOnHover(std::function<void(Element&)> callback) {
        onHover = std::move(callback);
    }

    void setOnUpdate(std::function<void(Element&, float dt)> callback) {
        onUpdate = std::move(callback);
    }

    // Function to be used for system.
    virtual void doUpdate(float dt) = 0;

    bool isPointColliding(const vec2& point) {
        vec2 minBounds = position - (scale + offset.x)*0.5f;
        vec2 maxBounds = position + (scale - offset.y)*0.5f;
        return point.x >= minBounds.x && point.x <= maxBounds.x &&
               point.y >= minBounds.y && point.y <= maxBounds.y;
    }

    void clickButton() {
        if (onClick) onClick(*this);
    }

    virtual ~Element() = default;

protected:
    std::function<void(Element&)> onHover;
    std::function<void(Element&)> onActive;
    std::function<void(Element&, float dt)> onUpdate;
    std::function<void(Element&)> onClick;
};

// This is just a wrapper for a list of UI Elements.
// This is also the Public API that will be used in our scenes.
class SceneUI {
public:
    bool insert(std::shared_ptr<Element> elem);
    bool remove(int idx);
    std::vector<std::shared_ptr<Element>> getElems();

    void update(float dt);
private: 
    std::vector<std::shared_ptr<Element>> ui_elems;
};

}
