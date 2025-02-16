#pragma once

#include "sceneManager/scene.hpp"
class MainMenuScene : public Scene {
public:
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void Update(float dt) override; 
};
