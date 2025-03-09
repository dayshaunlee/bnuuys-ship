#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "scenes/game_level.hpp"
#include "world_system.hpp"

class Level01 : public GameLevel {
private:
    // Runs after the GameLevel's init.
    void LevelInit() override;
    void LevelUpdate() override;
    void LevelExit() override;
    void LevelHandleInput(int key, int action, int mod) override;
    void LevelHandleMouseMove(glm::vec2 mousePos) override;
    void LevelHandleMouseClick(int button, int action, int mods) override;
    void LevelUpdate(float dt) override;

    // amount of bunnies player has to save to win
    int bunnies_to_win;
public:
    Level01(WorldSystem* worldsystem, std::string map_path);
    ~Level01() override;
}; 
