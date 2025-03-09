#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "scenes/game_level.hpp"
#include "world_system.hpp"

class TutorialLevel : public GameLevel {
private:
    // Runs after the GameLevel's init.
    void LevelInit() override;
    void LevelUpdate() override;
    void LevelExit() override;
    void LevelHandleInput(int key, int action, int mod) override;
    void LevelHandleMouseMove(glm::vec2 mousePos) override;
    void LevelHandleMouseClick(int button, int action, int mods) override;
    void LevelUpdate(float dt) override;

public:
    TutorialLevel(WorldSystem* worldsystem, std::string map_path);
    ~TutorialLevel() override;
}; 
