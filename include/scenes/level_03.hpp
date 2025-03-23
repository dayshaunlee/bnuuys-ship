#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
#include "world_system.hpp"
#include "saveload_system.hpp"

class Level03 : public GameLevel {
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
    Level03(WorldSystem* worldsystem, std::string map_path, TEXTURE_ASSET_ID texture);
    ~Level03() override;
}; 
