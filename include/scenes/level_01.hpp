#pragma once

#include <glm/ext/vector_float2.hpp>
#include "ai_system.hpp"
#include "animation_system.hpp"
#include "bnuui/bnuui.hpp"
#include "physics_system.hpp"
#include "sceneManager/scene.hpp"
#include "world_system.hpp"

class Level01 : public Scene {
private:
    AISystem ai_system;
    PhysicsSystem physics_system;
    AnimationSystem animation_system;
    WorldSystem world_system;

public:
    Level01();
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override; 
}; 
