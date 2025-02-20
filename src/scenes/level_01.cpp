#include "scenes/level_01.hpp"
#include <cmath>
#include <deque>
#include <glm/ext/vector_float2.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"
#include "map_init.hpp"

/*
*   Place 'local' scene vars here just so it's easy to manage.
*/

// Just to make my life easier. (TODO: REFACTOR LATER)
std::shared_ptr<bnuui::Cursor> tile_cursor;
vec2 l1_mouse_pos;

std::set<int> activeKeys;
std::deque<int> keyOrder;

std::set<int> activeShipKeys;
std::deque<int> keyShipOrder;

Level01::Level01() {
    this->name = "Level 1";
}

void Level01::Init() {
    // Probably where Dayshaun needs to preload the level 01 map.
    registry.list_all_components();
    std::cout << "loading map..." << std::endl;
    loadMap("m1.json");
    registry.list_all_components();

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

    tile_cursor = std::make_shared<bnuui::Cursor>(vec2(0,0), vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX), 0.0f);
    tile_cursor->visible = false;
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
    scene_ui.insert(progress_bar);
    scene_ui.insert(tile_cursor);
}

void Level01::Exit() {
    // Delete all components and stuff from this scene.
}

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

void HandleCameraMovement(int key, int action, int mod) {
    if (action == GLFW_PRESS) {
        activeShipKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        activeShipKeys.erase(key);
    }

    float accelerationX = 0.0f;
    float accelerationY = 0.0f;

    if (activeShipKeys.count(MOVE_UP_BUTTON)) accelerationY += SHIP_CAMERA_SPEED * 2;
    if (activeShipKeys.count(MOVE_DOWN_BUTTON)) accelerationY -= SHIP_CAMERA_SPEED * 2;
    if (activeShipKeys.count(MOVE_LEFT_BUTTON)) accelerationX += SHIP_CAMERA_SPEED * 2;
    if (activeShipKeys.count(MOVE_RIGHT_BUTTON)) accelerationX -= SHIP_CAMERA_SPEED * 2;

    CameraSystem::GetInstance()->setCameraScreen(accelerationX, accelerationY);
}

// tile_pos is the player's tile position when pressing SPACE.
void HandlePlayerStationing(vec2 tile_pos) {
    if (tile_pos.x < 0 || tile_pos.x >= COL_COUNT || tile_pos.y < 0 || tile_pos.y >= ROW_COUNT)
        return;

    Player& player_comp = registry.players.components[0];
    if (player_comp.player_state == STATIONING) {
        // Unstation and allow movement.
        player_comp.player_state = IDLE;
        return;
    }

    Ship& ship = registry.ships.components[0];
    MODULE_TYPES types = ship.ship_modules[tile_pos.y][tile_pos.x];
    switch (types) {
        case EMPTY:
        case PLATFORM:
            return;
        case STEERING_WHEEL:
        case SIMPLE_CANNON:
        case FAST_CANNON:
            player_comp.player_state = STATIONING;
    }
    player_comp.player_state = STATIONING;

}

void Level01::HandleInput(int key, int action, int mod) {
    Entity player = registry.players.entities[0];
    glm::vec2 playerPos = registry.motions.get(player).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player);

    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_SPACE) && 
        (player_comp.player_state == IDLE || player_comp.player_state == STATIONING)) {
        HandlePlayerStationing(vec2(player_tile_x, player_tile_y));
    }

    if (player_comp.player_state == STATIONING) {
        // Check what module the player is on.    
        Ship& ship = registry.ships.components[0];
        MODULE_TYPES module = ship.ship_modules[player_tile_y][player_tile_x];
        switch (module) {
            case EMPTY:
            case PLATFORM: {
                // This shouldn't happen but if it does, then we resolve by making the player back to idle.
                player_comp.player_state = IDLE;
                return;
            }
            case STEERING_WHEEL: {
                HandleCameraMovement(key, action, mod);
                return;
            }
            case SIMPLE_CANNON: {
                return;
            }
            case FAST_CANNON: {
                player_comp.player_state = IDLE;
                return;
            }
            break;
        }
    } else {
        HandlePlayerMovement(key, action, mod);
    } 
}

void Level01::HandleMouseMove(vec2 pos) {
    l1_mouse_pos = pos;
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
        if (ui_elem->isPointColliding(pos)) {
            ui_elem->hovering = true;
        } else {
            ui_elem->hovering = false;
        }
    }

    // Check if hovering over an interactable tile. IF SO, then place a cute cursor.
    int mouse_tile_x = (int) (pos.x / GRID_CELL_WIDTH_PX);
    int mouse_tile_y = (int) (pos.y / GRID_CELL_HEIGHT_PX);

    // Check if in the grid
    if (mouse_tile_x > 0 && mouse_tile_x < COL_COUNT && mouse_tile_y > 0 && mouse_tile_y < ROW_COUNT) {
        Ship& ship = registry.ships.components[0];
        MODULE_TYPES module = ship.ship_modules[mouse_tile_y][mouse_tile_x];
        if (module != EMPTY) {
            tile_cursor->visible = true;
            tile_cursor->position = TileToVector2(mouse_tile_x, mouse_tile_y);
        } else {
            tile_cursor->visible = false;
        }
    }

    Entity player_entity = registry.players.entities[0];
    glm::vec2 playerPos = registry.motions.get(player_entity).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player_entity);

    if (player_comp.player_state == STATIONING) {
        // Check what module the player is on.    
        Ship& ship = registry.ships.components[0];
        MODULE_TYPES module = ship.ship_modules[player_tile_y][player_tile_x];
        switch (module) {
            case SIMPLE_CANNON: {
                // Rotate the simple cannon.
                Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(cannon_entity);
                m.angle = degrees(atan2(pos.y-m.position.y, pos.x-m.position.x)) + 90.0f;
                return;
            }
            case FAST_CANNON: {
                player_comp.player_state = IDLE;
                return;
            }
            default: return;
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

    Entity player_entity = registry.players.entities[0];
    glm::vec2 playerPos = registry.motions.get(player_entity).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player_entity);

    if (player_comp.player_state == STATIONING) {
        // Check what module the player is on.    
        Ship& ship = registry.ships.components[0];
        MODULE_TYPES module = ship.ship_modules[player_tile_y][player_tile_x];
        switch (module) {
            case SIMPLE_CANNON: {
                // Rotate the simple cannon.
                Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                SimpleCannon& sc = registry.simpleCannons.get(cannon_entity);
                if (sc.timer_ms <= 0) {
                    vec2 cannon_pos = registry.motions.get(cannon_entity).position;
                    createCannonProjectile(cannon_pos, l1_mouse_pos);
                    sc.timer_ms = SIMPLE_CANNON_COOLDOWN;
                }
                return;
            }
            case FAST_CANNON: {
                player_comp.player_state = IDLE;
                return;
            }
            default: return;
        }
    }
}

void Level01::Update(float dt) {
    ai_system.step(dt);
    physics_system.step(dt);
    animation_system.step(dt);
    CameraSystem::GetInstance()->update(dt);

    // Simple cannon system. make this its own system later.
    for (SimpleCannon& sc : registry.simpleCannons.components) {
        if (sc.timer_ms > 0)
            sc.timer_ms -= dt;
        else sc.timer_ms = 0;
    }

    // Remove projectiles.
    for (Entity e : registry.projectiles.entities) {
        if (registry.projectiles.has(e)) {
            Projectile& p = registry.projectiles.get(e);
            if (p.alive_time_ms <= 0) {
                registry.remove_all_components_of(e);
                continue;
            }
            p.alive_time_ms -= dt;
        }
    }

    if (registry.players.components[0].health <= 0.0f)
        registry.players.components[0].health = 100;
    registry.players.components[0].health -= 0.1f;

    scene_ui.update(dt);
}
