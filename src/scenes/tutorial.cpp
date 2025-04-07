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
#include "camera_system.hpp"

TutorialLevel::TutorialLevel(WorldSystem* world_system, std::string map_filename, TEXTURE_ASSET_ID texture) : GameLevel(world_system) {
    this->name = "Tutorial Level";
    this->level_path = map_filename;
    this->texture = texture;
    GachaSystem::getInstance().setLevelPool(
        0, {MODULE_TYPES::SIMPLE_CANNON});
}

TutorialLevel::~TutorialLevel() {}

float x = 100.0f;

enum TUTORIAL_PHASE {
    WASD_KEYS,
    SPACEBAR_KEY,
    CANNON_SHOOT,
    STERRING_PAD,
    SAVE_BUNNY1,
    BUILD_MODE,
    AUTO_BUNNY,
    SAVE_BUNNY2,
    GOTO_BASE,
    ENEMIES_DAMAGE,
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
    // Create the press tutorial dialogue.
    auto tutorial_dialogue = std::make_shared<bnuui::DialogueBox>(
        vec2(600, 475), vec2(288, 96), 0.0f
    );

    // freetype doesnt support \n, probably should use a helper ;-;
    auto dialogue_txt_first = std::make_shared<bnuui::TextLabel>(vec2(480, 460), 1.0f, ":3");
    auto dialogue_txt_second = std::make_shared<bnuui::TextLabel>(vec2(480, 480), 1.0f, " ");

    dialogue_txt_first->setOnUpdate([](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            static_cast<bnuui::TextLabel&>(e).setText("Press WASD to move the player.");
        } else if (curr_tutorial_phase == SPACEBAR_KEY) {
            static_cast<bnuui::TextLabel&>(e).setText("Press space bar to interact");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            static_cast<bnuui::TextLabel&>(e).setText("Try using the steering pad");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            static_cast<bnuui::TextLabel&>(e).setText("Try using the cannon.");
        }  else if (curr_tutorial_phase == ENEMIES_MOVE) {
            static_cast<bnuui::TextLabel&>(e).setText("Enemies are approaching.");
        } else if (curr_tutorial_phase == ENEMIES_DAMAGE) {
            static_cast<bnuui::TextLabel&>(e).setText("Enemies can do damage to your");
        } else if (curr_tutorial_phase == SAVE_BUNNIES) {
            static_cast<bnuui::TextLabel&>(e).setText("Use the cannon to rescue");
        } else if (curr_tutorial_phase == GOTO_BASE) {
            static_cast<bnuui::TextLabel&>(e).setText("Bring them back to ");
        } else if (curr_tutorial_phase == BUILD_MODE) {
            static_cast<bnuui::TextLabel&>(e).setText("Press B to pause and enter");
        } 
    });

    dialogue_txt_second->setOnUpdate([](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == SPACEBAR_KEY) {
            static_cast<bnuui::TextLabel&>(e).setText("with/exit ship modules.");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            static_cast<bnuui::TextLabel&>(e).setText("to move the ship.");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            static_cast<bnuui::TextLabel&>(e).setText("Use left click to shoot.");
        } else if (curr_tutorial_phase == ENEMIES_MOVE) {
            static_cast<bnuui::TextLabel&>(e).setText("KILL THEM!!");
        } else if (curr_tutorial_phase == ENEMIES_DAMAGE) {
            static_cast<bnuui::TextLabel&>(e).setText("ship’s health. Be careful.");
        } else if (curr_tutorial_phase == SAVE_BUNNIES) {
            static_cast<bnuui::TextLabel&>(e).setText("jailed bunnies on islands.");
        } else if (curr_tutorial_phase == GOTO_BASE) {
            static_cast<bnuui::TextLabel&>(e).setText("highlighted area.");
        } else if (curr_tutorial_phase == BUILD_MODE) {
            static_cast<bnuui::TextLabel&>(e).setText("build mode.");
        } 
    });
    // Create a press spacebar
    scene_ui.insert(player_box);
    scene_ui.insert(tutorial_talk);
    scene_ui.insert(tutorial_dialogue);
    scene_ui.insert(dialogue_txt_first);
    scene_ui.insert(dialogue_txt_second);
    
}

void TutorialLevel::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void TutorialLevel::LevelExit() {}

void TutorialLevel::LevelHandleInput(int key, int action, int mod) {
    if (dialogue_timer_ms > 0) {
        return;
    } else if (curr_tutorial_phase == WASD_KEYS) {
        if (action == GLFW_PRESS && 
        (key == MOVE_UP_BUTTON || key == MOVE_DOWN_BUTTON || key == MOVE_LEFT_BUTTON || key == MOVE_RIGHT_BUTTON)) { 
            curr_tutorial_phase = SPACEBAR_KEY;
            dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    }
}

void TutorialLevel::LevelHandleMouseMove(vec2 pos) {}

void TutorialLevel::LevelHandleMouseClick(int button, int action, int mods) {}

void TutorialLevel::LevelUpdate(float dt) {
    Overlay& overlay = registry.overlays.components[0];
    overlay.visible = true;
    for (Entity entity : registry.enemies.entities) {
        Enemy& enemy = registry.enemies.get(entity);
        if (enemy.range != 3) {
            if (registry.walkingPaths.has(entity)) registry.walkingPaths.remove(entity);
            enemy.range = 3;
        }
        float diagonal = sqrt(WINDOW_WIDTH_PX * WINDOW_WIDTH_PX + WINDOW_HEIGHT_PX * WINDOW_HEIGHT_PX);
        RenderRequest& RR = registry.renderRequests.get(entity);
        RR.highlight_radius = 56 / diagonal;
    }

    if (dialogue_timer_ms > 0) {
        dialogue_timer_ms -= dt;
    } else if (curr_tutorial_phase == SPACEBAR_KEY) {
        if (registry.players.components[0].player_state == PLAYERSTATE::STATIONING) {
            curr_tutorial_phase = STERRING_PAD;
            dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == STERRING_PAD) {
        if (CameraSystem::GetInstance()->position.x != 0 || CameraSystem::GetInstance()->position.y != 0) {
            curr_tutorial_phase = CANNON_SHOOT;
            dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == CANNON_SHOOT) {
        if (registry.playerProjectiles.components.size() > 0) {
            for (Entity entity: registry.enemies.entities) {
                Enemy& enemy = registry.enemies.get(entity);
                enemy.speed = getEnemySpeed(enemy.type);
            }
            curr_tutorial_phase = ENEMIES_MOVE;
            dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == ENEMIES_MOVE) {
        if (registry.enemies.entities.size() == 0) {
            curr_tutorial_phase = ENEMIES_DAMAGE;
            dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == ENEMIES_DAMAGE) {
        if (registry.bunnies.components.size() > 0 || 
            registry.base.components[0].bunny_count > 0) {
                curr_tutorial_phase = SAVE_BUNNIES;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == SAVE_BUNNIES) {
        if (registry.bunnies.components.size() > 0 || 
            registry.base.components[0].bunny_count > 0) {
                curr_tutorial_phase = GOTO_BASE;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
        }
    } else if (curr_tutorial_phase == GOTO_BASE) {
        curr_tutorial_phase = BUILD_MODE;
        dialogue_timer_ms = DIALOGUE_TIME_MS;
    } else if (registry.base.components[0].bunny_count > 0) {
        // Skip tutorial.

        if(upgradesReceived == 1){
            SceneManager& sceneManager = SceneManager::getInstance();
            sceneManager.setNextLevelScence("Level 1");
            std::cout << "Switching to next level scene.." << std::endl;
            sceneManager.switchScene("Next Level Scene");

            // tutorial upgrades will not carry over to actual levels
            // SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
            // GameData gameData = saveLoadSystem.createGameData("Player1", "Level 1", registry.ships.components[0]);
            // saveLoadSystem.saveGame(gameData, "level_save.json");
            // std::cout << "tutorial saved" << std::endl;

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
