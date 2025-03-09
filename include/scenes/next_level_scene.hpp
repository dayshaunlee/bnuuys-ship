#pragma once

#include "sceneManager/scene.hpp"
class NextLevelScene : public Scene {
public:
    NextLevelScene();
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override;
};
