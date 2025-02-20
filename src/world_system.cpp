// Header
#include "world_system.hpp"
#include "GLFW/glfw3.h"
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "map_init.hpp"

// stlib
#include <cassert>
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <sstream>

#include <deque>

#include "physics_system.hpp"
#include "camera_system.hpp"

// create the world
WorldSystem::WorldSystem() {
    // seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
    // Destroy music components
    if (background_music != nullptr) Mix_FreeMusic(background_music);
    if (chicken_dead_sound != nullptr) Mix_FreeChunk(chicken_dead_sound);
    if (chicken_eat_sound != nullptr) Mix_FreeChunk(chicken_eat_sound);
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();

    // Close the window
    glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char* desc) {
    std::cerr << error << ": " << desc << std::endl;
}
}  // namespace

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow* WorldSystem::create_window() {
    ///////////////////////////////////////
    // Initialize GLFW
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
        return nullptr;
    }

    //-------------------------------------------------------------------------
    // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3
    // and enable the glDebugMessageCallback to have OpenGL catch your mistakes
    // for you. GLFW / OGL Initialization
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you
    // must handle different scalings glfwWindowHint(GLFW_SCALE_TO_MONITOR,
    // GL_TRUE);		// GLFW 3.3+
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);  // GLFW 3.3+

    // Create the main window (for rendering, keyboard, and mouse input)
    window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
        return nullptr;
    }

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(window, this);
    auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
        ((WorldSystem*) glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
    };
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
        ((WorldSystem*) glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
    };
    auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) {
        ((WorldSystem*) glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods);
    };

    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

    return window;
}

bool WorldSystem::start_and_load_sounds() {
    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
    chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
    chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

    if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
        fprintf(stderr,
                "Failed to load sounds\n %s\n %s\n %s\n make sure the data "
                "directory is present",
                audio_path("music.wav").c_str(),
                audio_path("chicken_dead.wav").c_str(),
                audio_path("chicken_eat.wav").c_str());
        return false;
    }

    return true;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;

    // start playing background music indefinitely
    std::cout << "Starting music..." << std::endl;
    Mix_PlayMusic(background_music, -1);

    // Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
    // Updating window title with points
    std::stringstream title_ss;
    glfwSetWindowTitle(window, title_ss.str().c_str());

    assert(registry.screenStates.components.size() <= 1);
    ScreenState& screen = registry.screenStates.components[0];
    return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
    std::cout << "Restarting..." << std::endl;

    // Debugging for memory/component leaks
    registry.list_all_components();

    // Remove all entities that we created
    // All that have a motion, we could also iterate over all bug, eagles, ... but
    // that would be more cumbersome
    while (registry.motions.entities.size() > 0) registry.remove_all_components_of(registry.motions.entities.back());

    // debugging for memory/component leaks
    registry.list_all_components();

    // create the ocean background and then ship
    createWaterBackground();
    createShip();
    // Now let's create our player.
    createPlayer(renderer, {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});

    int grid_line_width = GRID_LINE_WIDTH_PX;
    int CELL_WIDTH = WINDOW_WIDTH_PX / 15;
    int CELL_HEIGHT = CELL_WIDTH;

    // create grid lines if they do not already exist
    if (grid_lines.size() == 0) {
        // vertical lines
        int cell_width = CELL_WIDTH;
        for (int col = 0; col < 15 + 1; col++) {
            // width of 2 to make the grid easier to see
            grid_lines.push_back(
                createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
        }

        // horizontal lines
        int cell_height = CELL_HEIGHT;
        for (int col = 0; col < 11 + 1; col++) {
            // width of 2 to make the grid easier to see
            grid_lines.push_back(
                createGridLine(vec2(0, col * cell_height), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
        }
    }
    registry.list_all_components();
    std::cout << "loading map..." << std::endl;
    loadMap("m1.json");
    registry.list_all_components();
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
    ComponentContainer<Island>& island_container = registry.islands;
    int player_x = registry.motions.get(registry.players.entities[0]).position.x;
    int player_y = registry.motions.get(registry.players.entities[0]).position.y;
    DIRECTION player_direction = registry.players.get(registry.players.entities[0]).direction;
    PLAYERSTATE player_state = registry.players.get(registry.players.entities[0]).player_state;

    //if (player_state == WALKING) {
    //    if (player_direction == UP || player_direction == DOWN) {
    //        for (uint i = 0; i < island_container.components.size(); i++) {
    //            Island& island = island_container.components[i];
    //            if (island.polygon.size() > 0) {
    //                for (uint j = 0; j < island.polygon.size() - 1; j++) {
    //                    if (island.polygon[j].x == island.polygon[j + 1].x) {
    //                        if (((player_x >= island.polygon[j].x && player_x <= island.polygon[j + 1].x) ||
    //                             (player_x >= island.polygon[j + 1].x && player_x <= island.polygon[j].x)) &&
    //                            player_y == island.polygon[j].y) {
    //                            registry.motions.get(registry.players.entities[0]).velocity.y -= WALK_SPEED;
    //                            registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //                            break;
    //                        }
    //                    }
    //                }

    //                if (island.polygon[island.polygon.size() - 1].x == island.polygon[0].x) {
    //                    if (((player_x >= island.polygon[island.polygon.size() - 1].x &&
    //                          player_x <= island.polygon[0].x) ||
    //                         (player_x <= island.polygon[island.polygon.size() - 1].x &&
    //                          player_x >= island.polygon[0].x)) &&
    //                        player_y == island.polygon[0].y) {
    //                        registry.motions.get(registry.players.entities[0]).velocity.y -= WALK_SPEED;
    //                        registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //    } else if (player_direction == LEFT || player_direction == RIGHT) {
    //        for (uint i = 0; i < island_container.components.size(); i++) {
    //            Island& island = island_container.components[i];
    //            if (island.polygon.size() > 0) {
    //                for (uint j = 0; j < island.polygon.size() - 1; j++) {
    //                    if (island.polygon[j].y == island.polygon[j + 1].y) {
    //                        if (((player_y >= island.polygon[j].y && player_y <= island.polygon[j + 1].y) ||
    //                             (player_y >= island.polygon[j + 1].y && player_y <= island.polygon[j].y)) &&
    //                            player_x == island.polygon[j].x) {
    //                            registry.motions.get(registry.players.entities[0]).velocity.x -= WALK_SPEED;
    //                            registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //                            break;
    //                        }
    //                    }
    //                }
    //                if (island.polygon[island.polygon.size() - 1].y == island.polygon[0].y) {
    //                    if (((player_y >= island.polygon[island.polygon.size() - 1].y &&
    //                          player_y <= island.polygon[0].y) ||
    //                         (player_y <= island.polygon[island.polygon.size() - 1].y &&
    //                          player_y >= island.polygon[0].y)) &&
    //                        player_x == island.polygon[0].x) {
    //                        registry.motions.get(registry.players.entities[0]).velocity.x -= WALK_SPEED;
    //                        registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}

    //// Check if player is on the base
    //ComponentContainer<Base>& base_container = registry.base;
    //for (uint i = 0; i < base_container.components.size(); i++) {
    //    Base& base = base_container.components[i];
    //    if (base.polygon.size() > 0) {
    //        if (base.polygon[0].x < player_x && base.polygon[0].y > player_y && base.polygon[1].x > player_x &&
    //            base.polygon[1].y > player_y && base.polygon[2].x > player_x && base.polygon[2].y < player_y &&
    //            base.polygon[3].x < player_x && base.polygon[3].y < player_y && player_state == WALKING) {
    //            // handle collision on the ship
    //            if (player_direction == UP && player_y == base.polygon[0].y) {
    //                registry.motions.get(registry.players.entities[0]).velocity.y -= WALK_SPEED;
    //                registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //            } else if (player_direction == DOWN && player_y == base.polygon[2].y) {
    //                registry.motions.get(registry.players.entities[0]).velocity.y -= WALK_SPEED;
    //                registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //            } else if (player_direction == LEFT && player_x == base.polygon[3].x) {
    //                registry.motions.get(registry.players.entities[0]).velocity.x -= WALK_SPEED;
    //                registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //            } else if (player_direction == RIGHT && player_x == base.polygon[1].x) {
    //                registry.motions.get(registry.players.entities[0]).velocity.x -= WALK_SPEED;
    //                registry.players.get(registry.players.entities[0]).player_state = IDLE;
    //            }
    //        }
    //    }
    //}

    ComponentContainer<Collision>& collision_container = registry.collisions;
    for (uint i = 0; i < collision_container.components.size(); i++) {
        Entity e1 = collision_container.entities[i];
        Entity e2 = collision_container.components[i].other;

        /* TODO: implement projectiles from weapons
        if ((registry.projectiles.has(e1) && registry.enemies.has(e2)) ||
            (registry.projectiles.has(e2) && registry.enemies.has(e1))) {
            // do damage to enemies
            registry.remove_all_components_of(e1);
            registry.remove_all_components_of(e2);
        }
        */

        // Enemy - Ship collision
        if ((registry.enemies.has(e1) && registry.ships.has(e2)) ||
                 (registry.enemies.has(e2) && registry.ships.has(e1))) {
            if (registry.ships.has(e1)) { // e1 is the ship
                registry.ships.get(e1).health -= 10;
                registry.remove_all_components_of(e2);
            }
            else {  // e2 is the ship
                registry.ships.get(e2).health -= 10;
                registry.remove_all_components_of(e1);
            }
        }

        // Ship - Island collision
        if ((registry.ships.has(e1) && registry.islands.has(e2)) ||
            (registry.ships.has(e2) && registry.islands.has(e1))) {
            if (registry.ships.has(e1)) {  // e1 is the ship
                registry.ships.get(e1).health -= 10;
                registry.remove_all_components_of(e2);
            } else {  // e2 is the ship
                registry.ships.get(e2).health -= 10;
                registry.remove_all_components_of(e1);
            }
        }
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
    return bool(glfwWindowShouldClose(window));
}

std::set<int> activeKeys;
std::deque<int> keyOrder;
void HandlePlayerMovement(int key, int, int action, int mod) {
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
void HandleCameraMovement(int key, int, int action, int mod) {
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

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // exit game w/ ESC
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        close_window();
    }

    // Resetting game
    if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        restart_game();
    }

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
        HandleCameraMovement(key, 0, action, mod);
    } else {
        playerToChangeState.player_state = IDLE;
        HandlePlayerMovement(key, 0, action, mod);
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    // record the current mouse position
    mouse_pos_x = mouse_position.x;
    mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
    // on button press
    if (action == GLFW_PRESS) {
        int tile_x = (int) (mouse_pos_x / GRID_CELL_WIDTH_PX);
        int tile_y = (int) (mouse_pos_y / GRID_CELL_HEIGHT_PX);

        std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
        std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
    }
}
