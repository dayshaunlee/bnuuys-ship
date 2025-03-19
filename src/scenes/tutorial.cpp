#include "scenes/tutorial.hpp"
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <string>
#include "GLFW/glfw3.h"
#include "bnuui/buttons.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "scenes/game_level.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "gacha_system.hpp"
#include "saveload_system.hpp"

TutorialLevel::TutorialLevel(WorldSystem* world_system, std::string map_filename, TEXTURE_ASSET_ID texture) : GameLevel(world_system) {
    this->name = "Tutorial Level";
    this->level_path = map_filename;
    this->texture = texture;
    GachaSystem::getInstance().setLevelPool(
        0, {MODULE_TYPES::HELPER_BUNNY});
}

TutorialLevel::~TutorialLevel() {}

float x = 100.0f;

enum TUTORIAL_PHASE {
    WASD_KEYS,
    SPACEBAR_KEY,
    SAVE_BUNNIES,
    GOTO_BASE,
};

vec2 tutorial_mouse_pos;
TUTORIAL_PHASE curr_tutorial_phase = WASD_KEYS;

void TutorialLevel::LevelInit() {
    // Initialize Tutorial UI.
    auto tutorial_talk = std::make_shared<bnuui::PlayerStatus>(
        vec2(765, 540), vec2(-60, 60), 0.0f, x, x);
    auto player_box = std::make_shared<bnuui::Box>(vec2(765, 540), vec2(96, 96), 0.0f);
    // Create the press tutorial dialog.
    auto tutorial_dialog = std::make_shared<bnuui::Box>(
        vec2(650, 425), vec2(200, 200), 0.0f
    );
    tutorial_dialog->texture = TEXTURE_ASSET_ID::TUTORIAL_WASD_UI;

    tutorial_dialog->setOnUpdate([](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            e.texture = TEXTURE_ASSET_ID::TUTORIAL_WASD_UI;
        } else if (curr_tutorial_phase == SPACEBAR_KEY) {
            e.texture = TEXTURE_ASSET_ID::TUTORIAL_SPACE_UI;
        } else if (curr_tutorial_phase == SAVE_BUNNIES) {
            e.texture = TEXTURE_ASSET_ID::TUTORIAL_MOUSE_UI;
        } else if (curr_tutorial_phase == GOTO_BASE) {
            e.texture = TEXTURE_ASSET_ID::TUTORIAL_HOME_UI;
        }
    });
    // Create a press spacebar
    scene_ui.insert(player_box);
    scene_ui.insert(tutorial_talk);
    scene_ui.insert(tutorial_dialog);
}

void TutorialLevel::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void TutorialLevel::LevelExit() {}

void TutorialLevel::LevelHandleInput(int key, int action, int mod) {
    if (curr_tutorial_phase == WASD_KEYS) {
        if (action == GLFW_PRESS && 
        (key == MOVE_UP_BUTTON || key == MOVE_DOWN_BUTTON || key == MOVE_LEFT_BUTTON || key == MOVE_RIGHT_BUTTON)) { 
            curr_tutorial_phase = SPACEBAR_KEY;
        }
    }
}

void TutorialLevel::LevelHandleMouseMove(vec2 pos) {}

void TutorialLevel::LevelHandleMouseClick(int button, int action, int mods) {}

void TutorialLevel::LevelUpdate(float dt) {
    if (curr_tutorial_phase == SPACEBAR_KEY) {
        if (registry.players.components[0].player_state == PLAYERSTATE::STATIONING) {
            curr_tutorial_phase = SAVE_BUNNIES;
        }
    }

    if (curr_tutorial_phase == SAVE_BUNNIES) {
        if (registry.bunnies.components[0].on_ship) {
            curr_tutorial_phase = GOTO_BASE;
        }
    }

    if (registry.bunnies.components[0].on_base) {
        // Skip tutorial.

        if(upgradesReceived == 1){
            SceneManager& sceneManager = SceneManager::getInstance();
            sceneManager.setNextLevelScence("Level 1");
            std::cout << "Switching to next level scene.." << std::endl;
            sceneManager.switchScene("Next Level Scene");

            SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
            GameData gameData = saveLoadSystem.createGameData("Player1", "Level 1", registry.ships.components[0]);
            saveLoadSystem.saveGame(gameData, "level_save.json");
            std::cout << "tutorial saved" << std::endl;

            return;
        }

        if(!(this->gacha_called)){
            std::cout << "tutorial gacha pop" << std::endl;
            // std::cout << this->gacha_called << std::endl;
            this->gacha_called = true;
            GachaSystem::getInstance().displayGacha(0, this->scene_ui, *this);
            // std::cout << this->gacha_called << std::endl;
        }
    }
}
