#include "scenes/level_01.hpp"
#include <deque>
#include <iostream>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"

Level01::Level01() {
    this->name = "Level 1";
}

vec2 mPos;
void Level01::Init() {
    // Probably where Dayshaun needs to preload the level 01 map.
    createPlayer({100,100});

    // Initialize the UI.
    std::shared_ptr<bnuui::SquareButton> button = std::make_shared<bnuui::SquareButton>(
        vec2(100,100),  // Position
        vec2(100,100),  // Scale
        0.0f);          // Rotation

    button->setOnClick([](bnuui::Element& e) {
        std::cout << "Clicking the button\n";
    });

    button->setOnActive([](bnuui::Element&e) {
        // Get curr mouse pos.
        e.position = mPos;
    });

    scene_ui.insert(button);
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
    if (player_comp.player_state == PLAYERSTATE::STATIONING)
        return;

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
    mPos = pos;
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
        if (ui_elem->isPointInside(pos)) {
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

