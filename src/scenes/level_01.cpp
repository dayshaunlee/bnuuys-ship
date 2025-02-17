#include "scenes/level_01.hpp"
#include <deque>
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"

Level01::Level01() {
    this->name = "Level 1";
}

void Level01::Init() {
    // Probably where Dayshaun needs to preload the level 01 map.
    createPlayer({100, 100});
    InitializeUI();
}

void Level01::InitializeUI() {
    // Create Healthbar.
    auto player_box = std::make_shared<bnuui::Box>(vec2(48*2,48*2), vec2(48*2,48*2), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(vec2(48*2, 48*2), 
                                                               vec2(30*2, 30*2), 
                                                               0.0f,
                                                               registry.players.components[0].health
                                                               );
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(48*5.5f, 48*2),
                                                      vec2(48*5, 48*1.5f),
                                                      0.0f
                                                      );
    player_box->children.push_back(slider_bg);
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
}

void Level01::Exit() {
    // Delete all components and stuff from this scene.
}

std::set<int> activeKeys;
std::deque<int> keyOrder;
void HandlePlayerMovement(int key, int action, int mod) {
    Entity player = registry.players.entities[0];
    Player& player_comp = registry.players.get(player);
    Motion& mot = registry.motions.get(player);

    // Prevent player from moving when they're stationing.
    if (player_comp.player_state == PLAYERSTATE::STATIONING) return;

    if (action == GLFW_PRESS) {
        if (!activeKeys.count(key)) {
            keyOrder.push_back(key);
        }
        activeKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        activeKeys.erase(key);
        keyOrder.erase(std::remove(keyOrder.begin(), keyOrder.end(), key), keyOrder.end());
    }

    float velocityX = 0.0f;
    float velocityY = 0.0f;

    if (activeKeys.count(MOVE_UP_BUTTON)) velocityY -= WALK_SPEED;
    if (activeKeys.count(MOVE_DOWN_BUTTON)) velocityY += WALK_SPEED;
    if (activeKeys.count(MOVE_LEFT_BUTTON)) velocityX -= WALK_SPEED;
    if (activeKeys.count(MOVE_RIGHT_BUTTON)) velocityX += WALK_SPEED;

    velocityX = std::clamp(velocityX, -WALK_SPEED, WALK_SPEED);
    velocityY = std::clamp(velocityY, -WALK_SPEED, WALK_SPEED);

    mot.velocity = vec2(velocityX, velocityY);

    // Update the player state.
    if (activeKeys.empty()) {
        player_comp.player_state = PLAYERSTATE::IDLE;
    } else {
        player_comp.player_state = PLAYERSTATE::WALKING;

        // Determine direction based on last key pressed
        if (!keyOrder.empty()) {
            int lastKey = keyOrder.back();
            if (lastKey == MOVE_UP_BUTTON) {
                player_comp.direction = UP;
            } else if (lastKey == MOVE_DOWN_BUTTON) {
                player_comp.direction = DOWN;
            } else if (lastKey == MOVE_LEFT_BUTTON) {
                player_comp.direction = LEFT;
            } else if (lastKey == MOVE_RIGHT_BUTTON) {
                player_comp.direction = RIGHT;
            }
        }
    }
}

void Level01::HandleInput(int key, int action, int mod) {
    HandlePlayerMovement(key, action, mod);
}

void Level01::HandleMouseMove(vec2 pos) {
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
        if (ui_elem->isPointColliding(pos)) {
            ui_elem->hovering = true;
        } else {
            ui_elem->hovering = false;
        }
    }
}

void Level01::HandleMouseClick(int button, int action, int mods) {
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
        for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
            if (ui_elem->hovering) {
                ui_elem->active = true;
            } else {
                ui_elem->active = false;
            }
        }
    } else if (action == GLFW_RELEASE) {
        for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
            if (ui_elem->active) {
                ui_elem->clickButton();
            }
            ui_elem->active = false;
        }
    }
}

void Level01::Update(float dt) {
    ai_system.step(dt);
    physics_system.step(dt);
    animation_system.step(dt);

    scene_ui.update(dt);
}
