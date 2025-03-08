#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "ai_system.hpp"
#include "animation_system.hpp"
#include "physics_system.hpp"
#include "sceneManager/scene.hpp"
#include "world_system.hpp"

// This class describes a parent class for Gameplay Levels.
class GameLevel : public Scene {
protected:
    AISystem ai_system;
    PhysicsSystem physics_system;
    AnimationSystem animation_system;
    WorldSystem* world_system;

    void InitializeUI();
    std::string level_path; // TODO: Make a mapping for level_path and the background.
    
    virtual void LevelInit() = 0;
    virtual void LevelUpdate() = 0;
    virtual void LevelExit() = 0;
    virtual void LevelHandleInput(int key, int action, int mod) = 0;
    virtual void LevelHandleMouseMove(glm::vec2 mousePos) = 0;
    virtual void LevelHandleMouseClick(int button, int action, int mods) = 0;
    virtual void LevelUpdate(float dt) = 0;

public:
    GameLevel(WorldSystem* worldsystem);
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override; 

    virtual ~GameLevel() = default;
}; 
