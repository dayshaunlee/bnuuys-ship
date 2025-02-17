#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "bnuui/bnuui.hpp"
class Scene {
protected:
    std::string name;
    bnuui::SceneUI scene_ui;

public:
    virtual ~Scene() = default;
    virtual void Init() = 0;
    virtual void Exit() = 0;
    virtual void HandleInput(int key, int action, int mod) = 0;
    virtual void Update(float dt) = 0;
    virtual void HandleMouseMove(glm::vec2 mousePos) = 0;
    virtual void HandleMouseClick(int button, int action, int mods) = 0;

    bnuui::SceneUI getUIElems() {
        return scene_ui;
    }

    const std::string& getName() {
        return name;
    }
};
