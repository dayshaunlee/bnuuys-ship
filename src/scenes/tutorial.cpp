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
        0, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::LASER_WEAPON, MODULE_TYPES::HEAL, MODULE_TYPES::BUBBLE_MOD});
}

TutorialLevel::~TutorialLevel() {}

float x = 100.0f;

bool spawned_dummy = false;
void TutorialLevel::LevelInit() {
    spawned_dummy = false;
    bunnies_to_win = 3;
    curr_tutorial_phase = WASD_KEYS;
    // Initialize Tutorial UI.
    auto tutorial_talk = std::make_shared<bnuui::PlayerStatus>(
        vec2(765, 540), vec2(-120, 120), 0.0f, x, x, true);
    //auto player_box = std::make_shared<bnuui::Box>(vec2(765, 540), vec2(96, 96), 0.0f);
    // Create the press tutorial dialogue.
    auto tutorial_dialogue = std::make_shared<bnuui::DialogueBox>(
        vec2(420, 500), vec2(630, 96), 0.0f, true
    );

    // freetype doesnt support \n, probably should use a helper ;-;
    auto dialogue_txt_first = std::make_shared<bnuui::TextLabel>(vec2(150, 490), 1.0f, ":3", true);
    auto dialogue_txt_second = std::make_shared<bnuui::TextLabel>(vec2(150, 510), 1.0f, " ", true);

    // lock the base
    registry.base.components[0].locked = true;

    dialogue_txt_first->setOnUpdate([this](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            static_cast<bnuui::TextLabel&>(e).setText(
                "Hi Bnuuy! I'm your inner voice here to help you explore the sea.");
        } else if (curr_tutorial_phase == SPACEBAR_KEY) {
            static_cast<bnuui::TextLabel&>(e).setText("Stand on top of CANNON AND PRESS SPACE TO USE.");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            static_cast<bnuui::TextLabel&>(e).setText("Try using the cannon. LEFT CLICK TO SHOOT.");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            static_cast<bnuui::TextLabel&>(e).setText("Press Space again to unstation. Move to the steering wheel and use");
        } else if (curr_tutorial_phase == SAVE_BUNNY1) {
            static_cast<bnuui::TextLabel&>(e).setText("A BUNNY NEEDS YOUR HELP! DRIVE THERE AND SHOOT THE CAGE TO FREE IT.");
        } else if (curr_tutorial_phase == BUILD_MODE) {
            static_cast<bnuui::TextLabel&>(e).setText("Once a bunny is freed, you can hire them and enable auto-shoot.");
        } else if (curr_tutorial_phase == AUTO_BUNNY) {
            static_cast<bnuui::TextLabel&>(e).setText("IN INVENTORY, LEFT CLICK THE BUNNY ICON, THEN LEFT CLICK ON");
        } else if (curr_tutorial_phase == SAVE_BUNNY2) {
            static_cast<bnuui::TextLabel&>(e).setText("Press E again to exit build mode/inventory.");
        } else if (curr_tutorial_phase == DROPOFF) {
            static_cast<bnuui::TextLabel&>(e).setText("LETS DROP OFF THE BUNNYS AT THE BASE. REMEMBER TO UNASSIGN THEM!");
        } else if (curr_tutorial_phase == NEW_MODULE) {
            static_cast<bnuui::TextLabel&>(e).setText("OPEN BOOK TO SEE WHAT NEW MODULE DOES");
          
        } else if (curr_tutorial_phase == SAVE_BUNNY3) {
            static_cast<bnuui::TextLabel&>(e).setText("DROPPING OFF BUNNIES AT BASE WILL UNLOCK MORE WEAPONS.");
        } else if (curr_tutorial_phase == SAVE_BUNNY4) {
            static_cast<bnuui::TextLabel&>(e).setText("NOW, GO SAVE THE REST OF THE BUNNIES.");
        } 
    });

    dialogue_txt_second->setOnUpdate([this](bnuui::Element& e, float dt) {
        if (curr_tutorial_phase == WASD_KEYS) {
            static_cast<bnuui::TextLabel&>(e).setText("You can call me Rabbit. First, try moving with WASD.");
        } else if (curr_tutorial_phase == AUTO_BUNNY) {
            static_cast<bnuui::TextLabel&>(e).setText("THE CANNON TO ASSIGN. THIS WILL ENABLE AUTO-SHOOTING.");
        } else if (curr_tutorial_phase == BUILD_MODE) {
            static_cast<bnuui::TextLabel&>(e).setText("PRESS E TO OPEN BUILD MODE/INVENTORY.");
        } else if (curr_tutorial_phase == STERRING_PAD) {
            static_cast<bnuui::TextLabel&>(e).setText("it (press space). USE WASD AND MOVE TOWARDS THE BUNNY INDICATOR.");
        } else if (curr_tutorial_phase == CANNON_SHOOT) {
            static_cast<bnuui::TextLabel&>(e).setText("Try shooting at the dummy bunny.");
        } else if (curr_tutorial_phase == SAVE_BUNNY2) {
            static_cast<bnuui::TextLabel&>(e).setText("NOW TRY SAVING THE NEXT BUNNY WITH THE HELP OF YOUR NEW FRIEND.");
        } else if (curr_tutorial_phase == DROPOFF) {
            static_cast<bnuui::TextLabel&>(e).setText("PRESS E and RIGHT CLICK ON THE CANNON TO UNASSIGN THE BUNNY.");
        } else if (curr_tutorial_phase == SAVE_BUNNY3) {
            static_cast<bnuui::TextLabel&>(e).setText("PRESS E AND BUILD SOME OF YOUR NEW STATIONS.");
        } else if (curr_tutorial_phase == NEW_MODULE) {
            static_cast<bnuui::TextLabel&>(e).setText("LEFT CLICK TO OPEN AND CLOSE THE BOOK.");
        }
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
    float scaling_factor_x = GRID_CELL_WIDTH_PX / (float) 16;
    float scaling_factor_y = GRID_CELL_HEIGHT_PX / (float) 16;
    vec2 offset =
        vec2((WINDOW_WIDTH_PX / 2) - (144 * scaling_factor_x), (WINDOW_HEIGHT_PX / 2) - (328 * scaling_factor_y));
    Overlay& overlay = registry.overlays.components[0];
    switch (curr_tutorial_phase) {
        case SPACEBAR_KEY: {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            if (RenderSystem::isPaused) {
                overlay.visible = false;
            } else {
                overlay.visible = true;
            }
            if (!registry.spotlights.has(registry.players.entities[0])) {
                vec2 pos = registry.motions.get(registry.ships.entities[0]).position - vec2(0, GRID_CELL_HEIGHT_PX);
                registry.spotlights.insert(registry.players.entities[0], {pos + vec2(5.0, 0), 20.0});
            }
            if (registry.players.components[0].player_state == PLAYERSTATE::STATIONING) {
                curr_tutorial_phase = CANNON_SHOOT;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case CANNON_SHOOT: {
            if (registry.enemies.components.size() == 0 && !spawned_dummy) {
                Entity entity = Entity();
                EnemySpawner& ene = registry.enemySpawners.emplace(entity);
                ene.type = DUMMY;
                ene.home_island = 0;
                Motion& mot = registry.motions.emplace(entity);
                mot.position = {103 * scaling_factor_x + offset.x, 312 * scaling_factor_y + offset.y};
                createEnemy(entity);
                spawned_dummy = true;
                registry.remove_all_components_of(entity);
            }
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
            if (RenderSystem::isPaused) {
                overlay.visible = false;
            } else {
                overlay.visible = true;
            }
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
            if (registry.bunnies.components.size() == 0) {
                Entity entity = Entity();
                Bunny& bun = registry.bunnies.emplace(entity);
                Motion& mot = registry.motions.emplace(entity);
                mot.position = {344 * scaling_factor_x + offset.x, 344 * scaling_factor_y + offset.y};
                createBunny(entity);
            }
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            if (RenderSystem::isPaused) {
                overlay.visible = false;
            } else {
                overlay.visible = true;
            }
            if (!registry.spotlights.has(registry.players.entities[0])) {
                vec2 pos = registry.motions.get(registry.ships.entities[0]).position;
                registry.spotlights.insert(registry.players.entities[0], {pos + vec2(5.0, 0), 25.0});
            }
            if (CameraSystem::GetInstance()->position.x != 0 || CameraSystem::GetInstance()->position.y != 0) {
                curr_tutorial_phase = SAVE_BUNNY1;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case SAVE_BUNNY1: {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            assert(registry.bunnies.components.size() > 0);
            if (tracker_off_screen && CameraSystem::GetInstance()->position.y < 320 && registry.bunnies.components[0].is_jailed) {
                if (RenderSystem::isPaused) {
                    overlay.visible = false;
                } else {
                    overlay.visible = true;
                }
                if (!registry.spotlights.has(registry.players.entities[0])) {
                    vec2 pos = tracker_ui->position;
                    registry.spotlights.insert(registry.players.entities[0], {pos + vec2(140.0, 0), 25.0});
                } else if (registry.spotlights.has(registry.players.entities[0])) {
                    registry.spotlights.remove(registry.players.entities[0]);
                }
            } else {
                overlay.visible = false;
            }
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
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            overlay.visible = false;
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
            if (registry.bunnies.components.size() == 1) {
                Entity entity = Entity();
                Bunny& bun = registry.bunnies.emplace(entity);
                Motion& mot = registry.motions.emplace(entity);
                mot.position = {344 * scaling_factor_x + offset.x, 168 * scaling_factor_y + offset.y};
                createBunny(entity);

                Entity spawner_entity = Entity();
                EnemySpawner& ene = registry.enemySpawners.emplace(spawner_entity);
                ene.type = BASIC_GUNNER;
                ene.home_island = 0;
                Motion& spawner_mot = registry.motions.emplace(spawner_entity);
                spawner_mot.position = {320 * scaling_factor_x + offset.x, 192 * scaling_factor_y + offset.y};
                createEnemy(spawner_entity);
                registry.remove_all_components_of(spawner_entity);
            }
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
            registry.base.components[0].locked = false;
            if (registry.base.components[0].bunny_count >= 2) {
                curr_tutorial_phase = NEW_MODULE;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case NEW_MODULE: {
            for (Entity entity : registry.spotlights.entities) {
                registry.spotlights.remove(entity);
            }
            Overlay& overlay = registry.overlays.components[0];
            if (upgradesReceived == 2) {
                if (RenderSystem::isPaused) {
                    overlay.visible = false;
                } else {
                    overlay.visible = true;
                }
                if (!registry.spotlights.has(registry.players.entities[0])) {
                    vec2 pos = book_icon->position;
                    registry.spotlights.insert(registry.players.entities[0], {pos + vec2(90.0, 0), 40.0});
                }
            } else
                overlay.visible = false;
            if (this->book->visible) {
                curr_tutorial_phase = SAVE_BUNNY3;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case SAVE_BUNNY3: {
            overlay.visible = false;
            if (registry.ships.components[0].available_modules[SIMPLE_CANNON] == 0 &&
                registry.ships.components[0].available_modules[LASER_WEAPON] == 0 &&
                registry.ships.components[0].available_modules[HEAL] == 0 &&
                registry.ships.components[0].available_modules[BUBBLE_MOD] <= 1) {
                curr_tutorial_phase = SAVE_BUNNY4;
                dialogue_timer_ms = DIALOGUE_TIME_MS;
            }
            break;
        }
        case SAVE_BUNNY4: {
            if (registry.bunnies.components.size() == 0 && registry.base.components[0].bunny_count == 2) {
                Entity entity = Entity();
                Bunny& bun = registry.bunnies.emplace(entity);
                Motion& mot = registry.motions.emplace(entity);
                mot.position = {112 * scaling_factor_x + offset.x, 64 * scaling_factor_y + offset.y};
                createBunny(entity);

                Entity spawner_entity = Entity();
                EnemySpawner& ene = registry.enemySpawners.emplace(spawner_entity);
                ene.type = BASIC_GUNNER;
                ene.home_island = 0;
                Motion& spawner_mot = registry.motions.emplace(spawner_entity);
                spawner_mot.position = {160 * scaling_factor_x + offset.x, 64 * scaling_factor_y + offset.y};
                createEnemy(spawner_entity);
                spawner_mot.position.y = 96 * scaling_factor_y + offset.y;
                createEnemy(spawner_entity);
                registry.remove_all_components_of(spawner_entity);
            }
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
