#pragma once

#include <glm/ext/vector_float2.hpp>
#include "sceneManager/scene.hpp"

class EditorUI : public Scene {
private:
    void InitializeUI();

public:
    EditorUI();
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override; 
}; 
