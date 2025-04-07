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

void TutorialLevel::LevelInit() {
    curr_tutorial_phase = WASD_KEYS;
    /*if (!registry.spotlights.has(registry.ships.entities[0]))
        registry.spotlights.insert(registry.ships.entities[0], {{registry.motions.get(registry.ships.entities[0]).position}, 20.0});*/
    // Initialize Tutorial UI.
    auto tutorial_talk = std::make_shared<bnuui::PlayerStatus>(
        vec2(765, 540), vec2(-120, 120), 0.0f, x, x, true);
    //auto player_box = std::make_shared<bnuui::Box>(vec2(765, 540), vec2(96, 96), 0.0f);
    // Create the press tutorial dialogue.
    auto tutorial_dialogue = std::make_shared<bnuui::DialogueBox>(
        vec2(420, 500), vec2(630, 96), 0.0f, true
    );

    // freetype doesnt support \n, probably should use a helper ;-;
    auto dialogue_txt_first = std::make_shared<bnuui::TextLabel>(vec2(155, 490), 1.0f, ":3", true);
    auto dialogue_txt_second = std::make_shared<bnuui::TextLabel>(vec2(155, 510), 1.0f, " ", true);

    dialogue_txt_first->setOnUpdate([this](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            static_cast<bnuui::TextLabel&>(e).setText(
                "Hi Bnuuy! I'm your inner voice here to help you explore the sea.");
        } else if (curr_tutorial_phase == SPACEBAR_KEY) {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            overlay.visible = true;
            if (!registry.spotlights.has(registry.players.entities[0])) {
                vec2 pos = registry.motions.get(registry.ships.entities[0]).position - vec2(0, GRID_CELL_HEIGHT_PX);
                registry.spotlights.insert(registry.players.entities[0], {pos + vec2(5.0, 0), 20.0});
            }
            static_cast<bnuui::TextLabel&>(e).setText("GO TO CANNON AND PRESS SPACEBAR");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            if (!registry.spotlights.has(registry.players.entities[0])) {
                vec2 pos = registry.motions.get(registry.ships.entities[0]).position - vec2(0, GRID_CELL_HEIGHT_PX);
                registry.spotlights.insert(registry.players.entities[0], {pos + vec2(5.0, 0), 20.0});
            }
            for (Entity enemy : registry.enemies.entities) {
                if (!registry.spotlights.has(enemy) && registry.enemies.get(enemy).type == DUMMY)
                    registry.spotlights.insert(enemy, {registry.motions.get(enemy).position - vec2(30.0, 0), 30.0});
            }
            Overlay& overlay = registry.overlays.components[0];
            overlay.visible = true;
            static_cast<bnuui::TextLabel&>(e).setText("Try using the cannon. SHOOT DUMMY.");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            overlay.visible = true;
            if (!registry.spotlights.has(registry.players.entities[0])) {
                vec2 pos = registry.motions.get(registry.ships.entities[0]).position;
                registry.spotlights.insert(registry.players.entities[0], {pos + vec2(5.0, 0), 20.0});
            }
            static_cast<bnuui::TextLabel&>(e).setText("Try using the steering pad. MOVE TO BUNNY INDICATOR.");
        } else if (curr_tutorial_phase == SAVE_BUNNY1) {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            std::cout << CameraSystem::GetInstance()->position.x << std::endl;
            if (CameraSystem::GetInstance()->position.x > -250) {
                overlay.visible = true;
                if (!registry.spotlights.has(registry.players.entities[0])) {
                    vec2 pos = tracker_ui->position;
                    registry.spotlights.insert(registry.players.entities[0], {pos + vec2(140.0, 0), 15.0});
                } else if (registry.spotlights.has(registry.players.entities[0])) {
                    registry.spotlights.remove(registry.players.entities[0]);
                }
            }
            else {
                    overlay.visible = false;
            }
            
            static_cast<bnuui::TextLabel&>(e).setText("A BUNNY NEEDS YOUR HELP! DRIVE THERE AND SHOOT THE CAGE TO FREE IT.");
        } else if (curr_tutorial_phase == BUILD_MODE) {
            static_cast<bnuui::TextLabel&>(e).setText("PRESS B TO OPEN BUILD MODE. YOU CAN USE THIS TO REARRANGE YOUR STATIONS.");
        } else if (curr_tutorial_phase == AUTO_BUNNY) {
            static_cast<bnuui::TextLabel&>(e).setText("PUT BUNNY ON CANNON WITH LMB ON BUNNY IN INVENTORY. THEN LEFT CLICK THE");
        } else if (curr_tutorial_phase == SAVE_BUNNY2) {
            static_cast<bnuui::TextLabel&>(e).setText("NOW TRY SAVING THE NEXT BUNNY WITH THE HELP OF YOUR NEW FRIEND.");
        } else if (curr_tutorial_phase == DROPOFF) {
            static_cast<bnuui::TextLabel&>(e).setText("LETS DROP OFF THE BUNNYS AT THE BASE. REMEMBER TO UNSTATION THEM!");
        } else if (curr_tutorial_phase == NEW_MODULE) {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            if (upgradesReceived == 2) {
                overlay.visible = true;
                if (!registry.spotlights.has(registry.players.entities[0])) {
                    vec2 pos = book_icon->position;
                    registry.spotlights.insert(registry.players.entities[0], {pos + vec2(90.0, 0), 40.0});
                }
            } else overlay.visible = false;
            static_cast<bnuui::TextLabel&>(e).setText("OPEN BOOK TO SEE WHAT NEW MODULE DOES");
   
        } else if (curr_tutorial_phase == SAVE_BUNNY3) {
            Overlay& overlay = registry.overlays.components[0];
            overlay.visible = false;
            static_cast<bnuui::TextLabel&>(e).setText("GO SAVE THE REST OF THE BUNNIES");
        } 
    });

    dialogue_txt_second->setOnUpdate([this](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            static_cast<bnuui::TextLabel&>(e).setText("You can call me Rabbit. First, try moving with WASD.");
        } else if (curr_tutorial_phase == AUTO_BUNNY) {
            static_cast<bnuui::TextLabel&>(e).setText(" CANNON TO HAVE IT HELP SHOOT. YOU CAN REMOVE MODULES WITH RMB.");
        } /*else if (curr_tutorial_phase == SPACEBAR_KEY) {
            static_cast<bnuui::TextLabel&>(e).setText("with/exit ship modules. ");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            static_cast<bnuui::TextLabel&>(e).setText("Use left click to shoot.");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            static_cast<bnuui::TextLabel&>(e).setText("to move the ship.");
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
        } */
        else {
            static_cast<bnuui::TextLabel&>(e).setText(" ");
        }
    });
    // Create a press spacebar
    //scene_ui.insert(player_box);
    scene_ui.insert(tutorial_dialogue);
    scene_ui.insert(dialogue_txt_first);
    scene_ui.insert(dialogue_txt_second);
    scene_ui.insert(tutorial_talk);
    
}

void TutorialLevel::LevelUpdate() {}

// GameLevel should already handle clearing all components. So don't have to worry here.
void TutorialLevel::LevelExit() {}

void TutorialLevel::LevelHandleInput(int key, int action, int mod) {
    /*if (dialogue_timer_ms > 0) {
        return;
    } else */if (curr_tutorial_phase == WASD_KEYS) {
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

    switch (curr_tutorial_phase) {
        case SPACEBAR_KEY: {
            if (registry.players.components[0].player_state == PLAYERSTATE::STATIONING) {
                curr_tutorial_phase = CANNON_SHOOT;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case CANNON_SHOOT: {
            bool dummy_dead = true;
            for (Entity entity : registry.enemies.entities) {
                Enemy& enemy = registry.enemies.get(entity);
                if (enemy.type == ENEMY_TYPE::DUMMY) dummy_dead = false;
            }
            if (dummy_dead) {
                curr_tutorial_phase = STERRING_PAD;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case STERRING_PAD: {
            if (CameraSystem::GetInstance()->position.x != 0 || CameraSystem::GetInstance()->position.y != 0) {
                curr_tutorial_phase = SAVE_BUNNY1;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case SAVE_BUNNY1: {
            for (Entity entity : registry.bunnies.entities) {
                if (registry.bunnies.get(entity).on_ship) {
                    curr_tutorial_phase = BUILD_MODE;
                    dialogue_timer_ms = DIALOGUE_TIME_MS;
                    break;
                }
            }
            break;
        }
        case BUILD_MODE: {
            Player& player_comp = registry.players.get(registry.players.entities[0]);
            if (player_comp.player_state == BUILDING) {
                curr_tutorial_phase = AUTO_BUNNY;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case AUTO_BUNNY: {
            for (Entity entity : registry.bunnies.entities) {
                if (registry.bunnies.get(entity).on_module) {
                    curr_tutorial_phase = SAVE_BUNNY2;
                    dialogue_timer_ms = DIALOGUE_TIME_MS;
                    break;
                }
            }
            break;
        }
        case SAVE_BUNNY2: {
            // TODO CREATE ENEMIES HERE
            int bunnys_on_ship = 0;
            for (Entity entity : registry.bunnies.entities) {
                if (registry.bunnies.get(entity).on_ship || registry.bunnies.get(entity).on_module) {
                    bunnys_on_ship += 1;
                }
            }
            if (bunnys_on_ship >= 2) {
                curr_tutorial_phase = DROPOFF;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }

        case DROPOFF: {
            if (registry.base.components[0].bunny_count >= 2) {
                curr_tutorial_phase = NEW_MODULE;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case NEW_MODULE: {
            if (upgradesReceived == 2 && this->book->visible) {
                curr_tutorial_phase = SAVE_BUNNY3;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case SAVE_BUNNY3: {
            if (upgradesReceived == bunnies_to_win) {
                SceneManager& sceneManager = SceneManager::getInstance();
                sceneManager.setNextLevelScence("Level 1");
                std::cout << "Switching to next level scene.." << std::endl;
                sceneManager.switchScene("Next Level Scene");

                // tutorial upgrades will not carry over to actual levels
                // SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
                // GameData gameData = saveLoadSystem.createGameData("Player1", "Level 1",
                // registry.ships.components[0]); saveLoadSystem.saveGame(gameData, "level_save.json"); std::cout <<
                // "tutorial saved" << std::endl;

                return;
            }
            break;
        }
        default:
            // do nothing
            break;
    }

    if (this->upgradesReceived < registry.base.components[0].bunny_count) {
        if (!(this->gacha_called)) {
            std::cout << "tutorial gacha pop" << std::endl;
            // std::cout << this->gacha_called << std::endl;
            this->gacha_called = true;
            GachaSystem::getInstance().displayGacha(0, this->scene_ui, *this);
            // std::cout << this->gacha_called << std::endl;
        }
    }
    
}
