#include "scenes/level_01.hpp"
#include <cmath>
#include <deque>
#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
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
MODULE_TYPES curr_selected = EMPTY;

std::set<int> activeKeys;
std::deque<int> keyOrder;

std::set<int> activeShipKeys;
std::deque<int> keyShipOrder;

Level01::Level01() {
    this->name = "Level 1";
}

MODULE_TYPES getModuleType(vec2 tile_pos) {
    Ship& ship = registry.ships.components[0];
    return ship.ship_modules[tile_pos.y][tile_pos.x];
}

vec2 getMouseTilePosition() {
    int tile_x = (int) (l1_mouse_pos.x / GRID_CELL_WIDTH_PX);
    int tile_y = (int) (l1_mouse_pos.y / GRID_CELL_HEIGHT_PX);
    return vec2(tile_x, tile_y);
}

void Level01::Init() {
    // load map
    registry.list_all_components();
    std::cout << "loading map..." << std::endl;
    tson::Vector2i mapSize = loadMap("m1.json");
    registry.list_all_components();
    std::cout << mapSize.x << ", " << mapSize.y << std::endl;

    // create the ocean background and then ship
    createIslandBackground(mapSize.x, mapSize.y);
    createShip();

    // Now let's create our player.
    createPlayer({WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});

    createCamera();

    // enemy creation
    for (Entity entity: registry.enemies.entities) {
        createEnemy(entity);
    };

    // bunny creation
    createBunny({200, 200});

    registry.players.components[0].health = 100.0f;
    InitializeUI();
}

void Level01::InitializeUI() {
    // Create Healthbar.
    auto player_box = std::make_shared<bnuui::Box>(vec2(96, 96), vec2(96, 96), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(
        vec2(96, 96), vec2(60, 60), 0.0f, registry.ships.components[0].health, 100);
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(256, 96), vec2(240, 72), 0.0f);
    auto progress_bar = std::make_shared<bnuui::ProgressBar>(
        vec2(256, 93), vec2(180, 24), 0.0f, registry.ships.components[0].health, 100);
    player_box->children.push_back(slider_bg);

    // Create the tile cursor effect.
    tile_cursor = std::make_shared<bnuui::Cursor>(vec2(0, 0), vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX), 0.0f);
    tile_cursor->visible = false;

    // Create the inventory bar.
    auto inventory_slots = std::make_shared<bnuui::LongBox>(vec2(420, 550), vec2(240, 72), 0.0f);

    auto steering_wheels = std::make_shared<bnuui::Box>(vec2(340, 547.5f), vec2(40, 40), 0.0f);
    auto steering_wheel_selected = std::make_shared<bnuui::Cursor>(vec2(340, 547.5f), vec2(40, 40), 0.0f);
    steering_wheel_selected->visible = false;
    steering_wheels->children.push_back(steering_wheel_selected);
    steering_wheels->texture = TEXTURE_ASSET_ID::SQUARE_3_CLICKED;
    steering_wheels->setOnClick([](bnuui::Element& e) {
        if (curr_selected == STEERING_WHEEL) {
            curr_selected = EMPTY;
        } else {
            curr_selected = STEERING_WHEEL;
        }
    });
    steering_wheels->setOnUpdate([](bnuui::Element& e, float dt) {
        Ship& ship = registry.ships.components[0];
        if (ship.available_modules[STEERING_WHEEL] == 0) {
            e.color = vec3(0.5f, 0, 0);
        } else {
            e.color = vec3(1, 1, 1);
        }

        if (curr_selected == STEERING_WHEEL)
            e.children[0]->visible = true;
        else
            e.children[0]->visible = false;
    });

    auto cannons = std::make_shared<bnuui::Box>(vec2(380, 547.5f), vec2(40, 40), 0.0f);
    auto cannons_selected = std::make_shared<bnuui::Cursor>(vec2(380, 547.5f), vec2(40, 40), 0.0f);
    cannons_selected->visible = false;
    cannons->children.push_back(cannons_selected);
    cannons->texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01;
    cannons->setOnClick([](bnuui::Element& e) {
        if (curr_selected == SIMPLE_CANNON) {
            curr_selected = EMPTY;
        } else {
            curr_selected = SIMPLE_CANNON;
        }
    });
    cannons->setOnUpdate([](bnuui::Element& e, float dt) {
        Ship& ship = registry.ships.components[0];
        if (ship.available_modules[SIMPLE_CANNON] == 0) {
            e.color = vec3(0.5f, 0, 0);
        } else {
            e.color = vec3(1, 1, 1);
        }

        if (curr_selected == SIMPLE_CANNON)
            e.children[0]->visible = true;
        else
            e.children[0]->visible = false;
    });

    // Insert all the stuff.
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
    scene_ui.insert(progress_bar);
    scene_ui.insert(tile_cursor);

    scene_ui.insert(inventory_slots);
    scene_ui.insert(steering_wheels);
    scene_ui.insert(cannons);
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
        if (!activeShipKeys.count(key)) {
            keyShipOrder.push_back(key);
        }
        activeShipKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        activeShipKeys.erase(key);
        keyShipOrder.erase(std::remove(keyShipOrder.begin(), keyShipOrder.end(), key), keyShipOrder.end());
    }

    float velX = 0.0f;
    float velY = 0.0f;
    if (activeShipKeys.count(MOVE_UP_BUTTON)) velY += SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_DOWN_BUTTON)) velY -= SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_LEFT_BUTTON)) velX += SHIP_CAMERA_SPEED;
    if (activeShipKeys.count(MOVE_RIGHT_BUTTON)) velX -= SHIP_CAMERA_SPEED;

    velX = std::clamp(velX, -WALK_SPEED, WALK_SPEED);
    velY = std::clamp(velY, -WALK_SPEED, WALK_SPEED);

    CameraSystem::GetInstance()->vel = vec2(velX, velY);
}

// tile_pos is the player's tile position when pressing SPACE.
void HandlePlayerStationing(vec2 tile_pos) {
    if (tile_pos.x < 0 || tile_pos.x >= COL_COUNT || tile_pos.y < 0 || tile_pos.y >= ROW_COUNT) return;

    Player& player_comp = registry.players.components[0];
    if (player_comp.player_state == STATIONING) {
        // Unstation and allow movement.
        player_comp.player_state = IDLE;
        return;
    }

    MODULE_TYPES types = getModuleType(tile_pos);
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
        MODULE_TYPES module = getModuleType(vec2(player_tile_x, player_tile_y));
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
            default:
                return;
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
        MODULE_TYPES module = getModuleType(vec2(mouse_tile_x, mouse_tile_y));
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
        MODULE_TYPES module = getModuleType(vec2(player_tile_x, player_tile_y));
        switch (module) {
            case SIMPLE_CANNON: {
                // Rotate the simple cannon.
                Entity cannon_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(cannon_entity);
                m.angle = degrees(atan2(pos.y - m.position.y, pos.x - m.position.x)) + 90.0f;
                return;
            }
            case FAST_CANNON: {
                player_comp.player_state = IDLE;
                return;
            }
            default:
                return;
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

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
        // Handles mouse clicks for different stationings.
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
                default:
                    return;
            }
        }
    }
    // Place selected items.
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && curr_selected != EMPTY) {
        vec2 tile_pos = getMouseTilePosition();
        MODULE_TYPES module = getModuleType(tile_pos);
        Ship& ship = registry.ships.components[0];
        if (ship.available_modules[curr_selected] == 0) {
            return;
        }

        if (module == PLATFORM) {
            ship.ship_modules[tile_pos.y][tile_pos.x] = curr_selected;
            Entity e;
            switch (curr_selected) {
                case STEERING_WHEEL: {
                    e = createSteeringWheel(tile_pos);
                    break;
                }
                case SIMPLE_CANNON: {
                    e = createCannon(tile_pos);
                    break;
                }
                default:
                    break;
            }
            ship.ship_modules_entity[tile_pos.y][tile_pos.x] = e;
            ship.available_modules[curr_selected]--;
        }
    }

    // Remove on right click.
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_2) {
        vec2 tile_pos = getMouseTilePosition();
        MODULE_TYPES module = getModuleType(tile_pos);
        switch (module) {
            case EMPTY:
            case PLATFORM:
                return;
            default: {
                Ship& ship = registry.ships.components[0];
                ship.ship_modules[tile_pos.y][tile_pos.x] = PLATFORM;
                registry.remove_all_components_of(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                ship.available_modules[module]++;
            }
        }
    }
}

void Level01::Update(float dt) {
    CameraSystem::GetInstance()->update(dt);
    ai_system.step(dt);
    physics_system.step(dt);
    animation_system.step(dt);

    world_system.handle_collisions();
    /*std::cout << CameraSystem::GetInstance()->velocity.x << " " << CameraSystem::GetInstance()->velocity.y <<
     * std::endl;*/

    // Simple cannon system. make this its own system later.
    for (SimpleCannon& sc : registry.simpleCannons.components) {
        if (sc.timer_ms > 0)
            sc.timer_ms -= dt;
        else
            sc.timer_ms = 0;
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

    scene_ui.update(dt);
}
