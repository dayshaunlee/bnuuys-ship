#include "scenes/level_01.hpp"
#include <deque>
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"

Level01::Level01() {
    this->name = "Level 1";
}

void Level01::Init() {
    // Probably where Dayshaun needs to preload the level 01 map.

    // create the ocean background and then ship
    createWaterBackground();
    createShip();
    // Now let's create our player.
    createPlayer({250, 250}); 
    registry.players.components[0].health = 100.0f;
    InitializeUI();
}

void Level01::InitializeUI() {
    // Create Healthbar.

    auto player_box = std::make_shared<bnuui::Box>(vec2(96,96), vec2(96,96), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(vec2(96, 96), vec2(60, 60), 0.0f, 
                                                               registry.players.components[0].health, 100);
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(256, 96), vec2(240, 72),0.0f);
    auto progress_bar = std::make_shared<bnuui::ProgressBar>(vec2(256, 93), vec2(180, 24), 0.0f,
                                                             registry.players.components[0].health, 100);
    player_box->children.push_back(slider_bg);
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
    scene_ui.insert(progress_bar);
}

void Level01::Exit() {
    // Delete all components and stuff from this scene.
}

std::set<int> activeKeys;
std::deque<int> keyOrder;
void HandlePlayerMovement(int key, int action, int mod) {
    assert(registry.players.size() == 1);
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
    if (activeKeys.empty() || ((!activeKeys.count(MOVE_UP_BUTTON)) && (!activeKeys.count(MOVE_DOWN_BUTTON)) &&
                               (!activeKeys.count(MOVE_LEFT_BUTTON)) && (!activeKeys.count(MOVE_RIGHT_BUTTON)))) {
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

std::set<int> activeShipKeys;
std::deque<int> keyShipOrder;
void HandleCameraMovement(int key, int action, int mod) {
    if (!registry.players.components[0].is_sailing_ship) return;

    if (action == GLFW_PRESS) {
        if (!activeShipKeys.count(key)) {
            keyShipOrder.push_back(key);
        }
        activeShipKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        activeShipKeys.erase(key);
        keyShipOrder.erase(std::remove(keyShipOrder.begin(), keyShipOrder.end(), key), keyShipOrder.end());
    }

    float accelerationX = 0.0f;
    float accelerationY = 0.0f;

    if (activeShipKeys.count(MOVE_UP_BUTTON)) accelerationY += SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_DOWN_BUTTON)) accelerationY -= SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_LEFT_BUTTON)) accelerationX += SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_RIGHT_BUTTON)) accelerationX -= SHIP_CAMERA_SPEED;

    CameraSystem::GetInstance()->setCameraScreen(accelerationX, accelerationY);
}

void Level01::HandleInput(int key, int action, int mod) {
    Entity player = registry.players.entities[0];
    glm::vec2 playerPos = registry.motions.get(player).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& playerToChangeState = registry.players.get(player);
    // if player idle in the middle of the ship and press space then they are controling the ship movement/camera goes
    // with the ship
    if ((player_tile_x == MIDDLE_GRID_X) && (player_tile_y == MIDDLE_GRID_Y) && (action == GLFW_RELEASE) &&
        (key == GLFW_KEY_SPACE) &&
        (registry.players.get(player).player_state == IDLE ||
         registry.players.get(player).player_state == STATIONING)) {
        playerToChangeState.is_sailing_ship = !playerToChangeState.is_sailing_ship;
        std::cout << "changed control state" << std::endl;
    }

    if (playerToChangeState.is_sailing_ship) {
        playerToChangeState.player_state = STATIONING;
        HandleCameraMovement(key, action, mod);
    } else {
        playerToChangeState.player_state = IDLE;
        HandlePlayerMovement(key, action, mod);
    }
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
    CameraSystem::GetInstance()->update(dt);

    if (registry.players.components[0].health <= 0.0f)
        registry.players.components[0].health = 100.0f;
    registry.players.components[0].health -= 0.1f;

    scene_ui.update(dt);
}
