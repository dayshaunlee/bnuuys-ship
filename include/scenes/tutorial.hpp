#pragma once

#include <glm/ext/vector_float2.hpp>
#include <string>
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
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
 
    enum TUTORIAL_PHASE {
        WASD_KEYS,
        SPACEBAR_KEY,
        CANNON_SHOOT,
        STERRING_PAD,
        SAVE_BUNNY1,
        BUILD_MODE,
        AUTO_BUNNY,
        SAVE_BUNNY2,
        DROPOFF,
        NEW_MODULE,
        SAVE_BUNNY3,
        SAVE_BUNNY4,
    };

    vec2 tutorial_mouse_pos;
    TUTORIAL_PHASE curr_tutorial_phase = WASD_KEYS;

public:
    TutorialLevel(WorldSystem* worldsystem, std::string map_path, TEXTURE_ASSET_ID texture);
    ~TutorialLevel() override;
    float DIALOGUE_TIME_MS = 5000.f;
    float dialogue_timer_ms = DIALOGUE_TIME_MS;
}; 
