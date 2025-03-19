#include "scenes/game_level.hpp"
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
#include "inventory_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "bnuui/buttons.hpp"
#include "map_init.hpp"
#include "saveload_system.hpp"

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



GameLevel::GameLevel(WorldSystem* worldsystem) : inventory_system(scene_ui, curr_selected) {
    this->world_system = worldsystem;
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

void GameLevel::Init() {
    activeShipKeys.clear();
    scene_ui.clear();
    RenderSystem::isRenderingGacha = false;
    gacha_called = false;
    upgradesReceived = 0;
    // create player
    Entity player = createPlayer({WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});
    // load map
    registry.list_all_components();
    std::cout << "loading map..." << std::endl;
    std::pair<tson::Vector2i, tson::Vector2i> map = loadMap(level_path);
    tson::Vector2i mapSize = map.first;
    tson::Vector2i map_offset = map.second;
    registry.list_all_components();
    std::cout << mapSize.x << ", " << mapSize.y << std::endl;

    // create the ocean background and then ship
    createWaterBackground();
    createIslandBackground(mapSize.x, mapSize.y, map_offset.x, map_offset.y, texture);
    
    createShip();
    assert(registry.base.entities.size() > 0);
    // stored from top left -> clockwise
    base_corners = createBaseProgressLines(registry.base.entities[0]);

    if (SaveLoadSystem::getInstance().hasLoadedData) {
        GameData gd = SaveLoadSystem::getInstance().loadedGameData;
        Ship& ship = registry.ships.components[0];
        ship.health = gd.ship_health;
        ship.maxHealth = gd.ship_maxHealth;
        ship.ship_modules = gd.used_modules;
        ship.available_modules = gd.unused_modules;

        std::cout << "loaded saved ship" << std::endl;
    }

    // render player
    renderPlayer(player);

    createCamera();

    bunnies_to_win = 0;
    // bunny creation
    for (Entity entity : registry.bunnies.entities) {
        createBunny(entity);
        bunnies_to_win += 1;
    };
    for (Entity entity : registry.disasters.entities) {
        createDisaster(entity);
    }
    /*createDisaster({300, 100}, DISASTER_TYPE::TORNADO);
    createDisaster({300, 100}, DISASTER_TYPE::WHIRLPOOL);*/

    registry.players.components[0].health = 100.0f;
    InitializeUI();

    LevelInit();
}

void GameLevel::InitializeUI() {
    // Create Healthbar.
    auto player_box = std::make_shared<bnuui::Box>(vec2(96, 96), vec2(96, 96), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(
        vec2(96, 96), vec2(60, 60), 0.0f, registry.ships.components[0].health, registry.ships.components[0].maxHealth);
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(256, 96), vec2(240, 72), 0.0f);
    auto progress_bar = std::make_shared<bnuui::ProgressBar>(
        vec2(256, 93), vec2(180, 24), 0.0f, registry.ships.components[0].health, registry.ships.components[0].maxHealth);
    player_box->children.push_back(slider_bg);

    // Create the tile cursor effect.
    tile_cursor = std::make_shared<bnuui::Cursor>(vec2(0, 0), vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX), 0.0f);
    tile_cursor->visible = false;


    // Insert all the stuff.
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);
    scene_ui.insert(progress_bar);
    scene_ui.insert(tile_cursor);
}

void GameLevel::Exit() {
    scene_ui.clear();
    CameraSystem* cs = CameraSystem::GetInstance();
    cs->position = {0.0f, 0.0f};
    cs->prev_pos = {0.0f, 0.0f};
    cs->vel = {0.0f, 0.0f};
    // Delete all components and stuff from this scene.

    while (registry.cameras.entities.size() > 0){
	    registry.remove_all_components_of(registry.cameras.entities.back());
	}
    while (registry.renderRequests.entities.size() > 0){
	    registry.remove_all_components_of(registry.renderRequests.entities.back());
	}
    while (registry.renderLayers.entities.size() > 0) {
        registry.remove_all_components_of(registry.renderLayers.entities.back());
    }
    while (registry.colors.entities.size() > 0){
	    registry.remove_all_components_of(registry.colors.entities.back());
	}
    while (registry.players.entities.size() > 0){
	    registry.remove_all_components_of(registry.players.entities.back());
	}
    while (registry.motions.entities.size() > 0){
	    registry.remove_all_components_of(registry.motions.entities.back());
	}
    while (registry.playerAnimations.entities.size() > 0){
	    registry.remove_all_components_of(registry.playerAnimations.entities.back());
	}
    while (registry.ships.entities.size() > 0){
	    registry.remove_all_components_of(registry.ships.entities.back());
	}
    while (registry.backgroundObjects.entities.size() > 0){
	    registry.remove_all_components_of(registry.backgroundObjects.entities.back());
	}
    while (registry.enemies.entities.size() > 0){
	    registry.remove_all_components_of(registry.enemies.entities.back());
	}
    while (registry.enemySpawners.entities.size() > 0) {
        registry.remove_all_components_of(registry.enemySpawners.entities.back());
    }
    while (registry.islands.entities.size() > 0){
	    registry.remove_all_components_of(registry.islands.entities.back());
	}
    while (registry.base.entities.size() > 0){
	    registry.remove_all_components_of(registry.base.entities.back());
	}
    while (registry.bunnies.entities.size() > 0){
	    registry.remove_all_components_of(registry.bunnies.entities.back());
	}
    while (registry.walkingPaths.entities.size() > 0){
        registry.remove_all_components_of(registry.walkingPaths.entities.back());
    }
    while (registry.filledTiles.entities.size() > 0){
        registry.remove_all_components_of(registry.filledTiles.entities.back());
    }
    while (registry.steeringWheels.entities.size() > 0){
        registry.remove_all_components_of(registry.steeringWheels.entities.back());
    }
    while (registry.simpleCannons.entities.size() > 0){
        registry.remove_all_components_of(registry.simpleCannons.entities.back());
    }
    while (registry.sounds.entities.size() > 0) {
        registry.remove_all_components_of(registry.sounds.entities.back());
    }
    // while (registry.projectiles.entities.size() > 0){
    //     registry.remove_all_components_of(registry.projectiles.entities.back());
    // }
    LevelExit();
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

void UpdateCameraMoveDirection(int key, int action, int mod){
    if (action == GLFW_PRESS) {
        if (!activeShipKeys.count(key)) {
            keyShipOrder.push_back(key);
        }
        activeShipKeys.insert(key);
        Camera& c  = registry.cameras.components[0];
        if (activeShipKeys.count(MOVE_UP_BUTTON) || activeShipKeys.count(MOVE_DOWN_BUTTON)) c.applyFrictionY = false;
        if (activeShipKeys.count(MOVE_LEFT_BUTTON) || activeShipKeys.count(MOVE_RIGHT_BUTTON)) c.applyFrictionX = false;
    } else if (action == GLFW_RELEASE) {
        activeShipKeys.erase(key);
        keyShipOrder.erase(std::remove(keyShipOrder.begin(), keyShipOrder.end(), key), keyShipOrder.end());
    }

    // If no keys are pressed, don't reset velocity
    if (activeShipKeys.empty() || (activeShipKeys.size() == 1 && activeShipKeys.count(GLFW_KEY_SPACE))) return;   
}

void HandleCameraMovement() {
    if (activeShipKeys.empty() || (activeShipKeys.size() == 1 && activeShipKeys.count(GLFW_KEY_SPACE))) return;
    CameraSystem* camera = CameraSystem::GetInstance();

    vec2 inputVel = {0.0f, 0.0f};

    if (activeShipKeys.count(MOVE_UP_BUTTON)) inputVel.y += 1;
    if (activeShipKeys.count(MOVE_DOWN_BUTTON)) inputVel.y -= 1;
    if (activeShipKeys.count(MOVE_LEFT_BUTTON)) inputVel.x += 1;
    if (activeShipKeys.count(MOVE_RIGHT_BUTTON)) inputVel.x -= 1;

    if(inputVel.x != 0 || inputVel.y != 0){
        float length = std::sqrt(inputVel.x* inputVel.x + inputVel.y * inputVel.y);
        if (length > 0){
            inputVel.x /= length;
            inputVel.y /= length;
        }
    }

    inputVel.x *= SHIP_CAMERA_SPEED;
    inputVel.y *= SHIP_CAMERA_SPEED;

    inputVel.x = std::clamp(inputVel.x, -WALK_SPEED, WALK_SPEED);
    inputVel.y = std::clamp(inputVel.y, -WALK_SPEED, WALK_SPEED);

    // Apply input to velocity directly instead of setting it
    camera->vel = inputVel;
}

// tile_pos is the player's tile position when pressing SPACE.
void HandlePlayerStationing(vec2 tile_pos) {
    activeShipKeys.clear();
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
        case LASER_WEAPON:
            player_comp.player_state = STATIONING;
    }
    player_comp.player_state = STATIONING;
}

void GameLevel::HandleInput(int key, int action, int mod) {
    Entity player = registry.players.entities[0];

    glm::vec2 playerPos = registry.motions.get(player).position;
    int player_tile_x = (int) (playerPos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int) (playerPos.y / GRID_CELL_HEIGHT_PX);

    Player& player_comp = registry.players.get(player);

    // Build Mode.
    if ((action == GLFW_RELEASE) && (key == GLFW_KEY_B) && (player_comp.player_state != STATIONING)) {
        if (player_comp.player_state == BUILDING) {
            inventory_system.CloseInventory();
            player_comp.player_state = IDLE;
        } else {
            player_comp.player_state = BUILDING;
            inventory_system.OpenInventory();
        }
        return;
    }

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
                UpdateCameraMoveDirection(key, action, mod);
                return;
            }
            default:
                return;
        }
    } else if (player_comp.player_state != BUILDING) {
        HandlePlayerMovement(key, action, mod);
    }

    LevelHandleInput(key, action, mod);
}

void GameLevel::HandleMouseMove(vec2 pos) {
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
            case LASER_WEAPON: {
                Entity laser_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                Motion& m = registry.motions.get(laser_entity);
                m.angle = degrees(atan2(pos.y - m.position.y, pos.x - m.position.x)) + 90.0f;
                return; 
            }
            default:
                return;
        }
    }

    LevelHandleMouseMove(pos);
}

// Stations the first bunny that's unstationed.
void stationBunny() {
    // Set one of the bunny to be on_module
    for (Bunny& b : registry.bunnies.components) {
        if (b.on_ship && !b.on_module && !b.on_base) {
            b.on_ship = false;
            b.on_module = true;
            return; 
        }
    }
}

// Unstations the frist stationed bunny.
void unStationBunny() {
    // Set one of the bunny to be on_module
    for (Bunny& b : registry.bunnies.components) {
        if (b.on_module) {
            b.on_ship = true;
            b.on_module = false;
            return; 
        }
    }
}

void GameLevel::HandleMouseClick(int button, int action, int mods) {
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
                case LASER_WEAPON: {
                    Entity laser_entity = ship.ship_modules_entity[player_tile_y][player_tile_x];
                    LaserWeapon& lw = registry.laserWeapons.get(laser_entity);
                    if (lw.timer_ms <= 0) {
                        vec2 laser_pos = registry.motions.get(laser_entity).position;
                        createLaserBeam(laser_pos, l1_mouse_pos);
                        lw.timer_ms = SIMPLE_CANNON_COOLDOWN;
                    }
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

        if (module == PLATFORM && curr_selected != HELPER_BUNNY) {
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
                case LASER_WEAPON: {
                    e = createLaserWeapon(tile_pos);
                    break;
                }
                default:
                    break;
            }
            ship.ship_modules_entity[tile_pos.y][tile_pos.x] = e;
            ship.available_modules[curr_selected]--;
        } else if (curr_selected == HELPER_BUNNY && module != EMPTY && module != PLATFORM) {
            switch (module) {
                case SIMPLE_CANNON: {
                    SimpleCannon& sc = registry.simpleCannons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!sc.is_automated) {
                        sc.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny();
                    }
                    break;
                }
                case LASER_WEAPON: {
                    LaserWeapon& lw = registry.laserWeapons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                    if (!lw.is_automated) {
                        lw.is_automated = true;
                        ship.available_modules[curr_selected]--;
                        stationBunny();
                    }
                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

    // Remove on right click.
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_2 && registry.players.components[0].player_state == BUILDING) {
        vec2 tile_pos = getMouseTilePosition();
        MODULE_TYPES module = getModuleType(tile_pos);
        Ship& ship = registry.ships.components[0];
        switch (module) {
            case EMPTY:
            case PLATFORM:
                return;
            case SIMPLE_CANNON: {
                SimpleCannon& sc = registry.simpleCannons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                if (sc.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    sc.is_automated = false;
                    unStationBunny();
                    break;
                }
                RemoveStation(tile_pos, module); 
                break;
            }
            case LASER_WEAPON: {
                LaserWeapon& lw = registry.laserWeapons.get(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
                if (lw.is_automated) {
                    ship.available_modules[HELPER_BUNNY]++;
                    lw.is_automated = false;
                    unStationBunny();
                    break;
                }
                RemoveStation(tile_pos, module);
                break;
            }
            default: {
                RemoveStation(tile_pos, module);
            }
        }
        // SUPER LAZY AND BAD WAY BUT IM LAZYYYY
        inventory_system.CloseInventory();
        inventory_system.OpenInventory();
    }

    LevelHandleMouseClick(button, action, mods);
}


/* Update the four lines of the progress bar
* 
*   when a side is full, we add half the width to the length
    so there are no gaps in the corners

    setting end_pos to (0, 0) effectively makes it invisible
*/
void GameLevel::UpdateDropoffProgressBar() {
    assert(registry.base.entities.size() == 1 && base_corners.size() == 4);
    Entity& base_entity = registry.base.entities[0];
    Motion& base_motion = registry.motions.get(base_entity);

    // percentage value from 0 to 1 of dropoff time
    float dropoffProgress = registry.base.get(base_entity).drop_off_timer / BUNNY_BASE_DROPOFF_TIME;

    float base_width = base_motion.scale.x, base_height = base_motion.scale.y;
    float base_perimeter = 2 * (base_width + base_height);

    std::vector<tson::Vector2i> original_corners = get_poly_from_motion(base_motion);

    GridLine& lineUL = registry.gridLines.get(base_corners[0]);
    GridLine& lineUR = registry.gridLines.get(base_corners[1]);
    GridLine& lineBR = registry.gridLines.get(base_corners[2]);
    GridLine& lineBL = registry.gridLines.get(base_corners[3]);

    auto setLine = [](GridLine& line, vec2 start, vec2 end) {
        line.start_pos = start;
        line.end_pos = end;
    };

    // percentage thresholds for each side
    float thresholds[] = {base_width / base_perimeter,
                          (base_width + base_height) / base_perimeter,
                          (2 * base_width + base_height) / base_perimeter,
                          1.0f};

    float new_length = dropoffProgress * base_perimeter;

    setLine(lineUL,
            {original_corners[0].x + std::min(new_length, base_width) / 2, original_corners[0].y},
            {std::min(new_length, base_width) + PROGRESS_BAR_LINE_WIDTH_PX / 2, PROGRESS_BAR_LINE_WIDTH_PX});
    setLine(
        lineUR,
        (dropoffProgress > thresholds[0])
            ? vec2(original_corners[1].x, original_corners[1].y + std::min(new_length - base_width, base_height) / 2)
            : vec2(0, 0),
        (dropoffProgress > thresholds[0])
            ? vec2(PROGRESS_BAR_LINE_WIDTH_PX, std::min(new_length - base_width, base_height))
            : vec2(0, 0));
    setLine(lineBR,
            (dropoffProgress > thresholds[1])
                ? vec2(original_corners[2].x - std::min(new_length - base_width - base_height, base_width) / 2,
                       original_corners[2].y)
                : vec2(0, 0),
            (dropoffProgress > thresholds[1])
                ? vec2(std::min(new_length - base_width - base_height, base_width) + PROGRESS_BAR_LINE_WIDTH_PX / 2,
                       PROGRESS_BAR_LINE_WIDTH_PX)
                : vec2(0, 0));
    setLine(lineBL,
            (dropoffProgress > thresholds[2])
                ? vec2(original_corners[3].x,
                       original_corners[3].y - std::min(new_length - 2 * base_width - base_height, base_height) / 2)
                : vec2(0, 0),
            (dropoffProgress > thresholds[2])
                ? vec2(PROGRESS_BAR_LINE_WIDTH_PX, std::min(new_length - 2 * base_width - base_height, base_height))
                : vec2(0, 0));
}


void GameLevel::RemoveStation(vec2 tile_pos, MODULE_TYPES module){
    Ship& ship = registry.ships.components[0];
    ship.ship_modules[tile_pos.y][tile_pos.x] = PLATFORM;
    registry.remove_all_components_of(ship.ship_modules_entity[tile_pos.y][tile_pos.x]);
    ship.available_modules[module]++; 
}


void GameLevel::Update(float dt) {
    if(!RenderSystem::isRenderingGacha && registry.players.components[0].player_state != BUILDING){
        CameraSystem::GetInstance()->update(dt);
        ai_system.step(dt);
        physics_system.step(dt);
        animation_system.step(dt);
        module_system.step(dt);
        HandleCameraMovement();

        world_system->handle_collisions();

        // Remove projectiles.
        for (Entity e : registry.playerProjectiles.entities) {
            if (registry.playerProjectiles.has(e)) {
                PlayerProjectile& p = registry.playerProjectiles.get(e);
                if (p.alive_time_ms <= 0) {
                    registry.remove_all_components_of(e);
                    continue;
                }
                p.alive_time_ms -= dt;
            }
        }

        for (Entity e : registry.enemyProjectiles.entities) {
            if (registry.enemyProjectiles.has(e)) {
                EnemyProjectile& p = registry.enemyProjectiles.get(e);
                if (p.alive_time_ms <= 0) {
                    registry.remove_all_components_of(e);
                    continue;
                }
                p.alive_time_ms -= dt;
            }
        }

        for (Entity e : registry.laserBeams.entities) {
            if (registry.laserBeams.has(e)) {
                LaserBeam& l = registry.laserBeams.get(e);
                if (l.alive_time_ms <= 0) {
                    registry.remove_all_components_of(e);
                    continue;
                }
                l.alive_time_ms -= dt;
            }
        }

        // Remove disasters.
        for (Entity e : registry.disasters.entities) {
            Disaster& d = registry.disasters.get(e);
            if (d.type == DISASTER_TYPE::TORNADO) {
                if (d.alive_time_ms <= 0) {
                    std::cout << "remove tornado" << std::endl;
                    registry.remove_all_components_of(e);
                    continue;
                }
                d.alive_time_ms -= dt;
            }
        }

        registry.ships.components[0].available_modules[HELPER_BUNNY] = 0;
        for (Bunny& bunny : registry.bunnies.components) {
            if (bunny.on_ship) {
                registry.ships.components[0].available_modules[HELPER_BUNNY]++;
            }
        }
    }

    UpdateDropoffProgressBar();

    scene_ui.update(dt);

    LevelUpdate(dt);
}
