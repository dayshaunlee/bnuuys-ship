// Header
#include "world_system.hpp"
#include "GLFW/glfw3.h"
#include "camera_system.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "map_init.hpp"

// stlib
#include <cassert>
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <ostream>
#include <sstream>

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
    // TODO Brian: uncomment later
    /*Mix_PlayMusic(background_music, -1);*/

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
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
    ComponentContainer<Collision>& collision_container = registry.collisions;
    std::vector<Entity> collisions_to_remove;

    for (uint i = 0; i < collision_container.components.size(); i++) {
        // Handle collision here.
        Entity entity_i = collision_container.entities[i];
        Entity entity_j = collision_container.components[i].other;

        Entity bunny_entity;

        if (registry.bunnies.has(entity_i)) {
            bunny_entity = entity_i;
        } else if (registry.bunnies.has(entity_j)) {
            bunny_entity = entity_j;
        } else {
            continue;
        }

        Entity other_entity = (bunny_entity == entity_i) ? entity_j : entity_i;

        Bunny& bunny = registry.bunnies.get(bunny_entity);

        if (registry.projectiles.has(other_entity)) {
            Projectile& projectile = registry.projectiles.get(other_entity);
            float bunny_hit = bunny.jail_health - projectile.damage;

            if (bunny.is_jailed) {
                if (bunny_hit <= 0) {
                    TEXTURE_ASSET_ID& texture = registry.renderRequests.get(bunny_entity).used_texture;
                    texture = TEXTURE_ASSET_ID::BUNNY_NOT_JAILED;
                    bunny.is_jailed = false;

                    registry.remove_all_components_of(other_entity);
                    registry.backgroundObjects.remove(bunny_entity);
                } else {
                    registry.remove_all_components_of(other_entity);
                    bunny.jail_health = bunny_hit;
                    std::cout << "projectile hit" << std::endl;
                    std::cout << "bunny jail health is now " << bunny_hit << std::endl;
                }
            }
        }
    }

    for (uint i = 0; i < collision_container.components.size(); i++) {
        Entity e1 = collision_container.entities[i];
        Entity e2 = collision_container.components[i].other;
        if (!registry.motions.has(e1) || !registry.motions.has(e2)) {
            collisions_to_remove.push_back(e1);
            collisions_to_remove.push_back(e2);
            continue;
        }
        /* TODO: implement projectiles from weapons
        if ((registry.projectiles.has(e1) && registry.enemies.has(e2)) ||
            (registry.projectiles.has(e2) && registry.enemies.has(e1))) {
            // do damage to enemies
            registry.remove_all_components_of(e1);
            registry.remove_all_components_of(e2);
        }
        */

        // Enemy - Ship collision
        if (registry.enemies.has(e1) && registry.ships.has(e2)) {
            registry.ships.get(e2).health -= 10.0f;
            registry.remove_all_components_of(e1);
            continue;
        } else if (registry.enemies.has(e2) && registry.ships.has(e1)) {
            registry.ships.get(e1).health -= 10.0f;
            registry.remove_all_components_of(e2);
            continue;
        }

        // Ship - Island collision
        if ((registry.ships.has(e1) && registry.islands.has(e2)) ||
            (registry.ships.has(e2) && registry.islands.has(e1))) {
            // debugging only right now
            int ship_x;
            int ship_y;
            int island_x;
            int island_y;

            collisions_to_remove.push_back(e1);
            collisions_to_remove.push_back(e2);
            if (registry.ships.has(e1)) {  // e1 is the ship
                ship_x = registry.motions.get(e1).position.x;
                ship_y = registry.motions.get(e1).position.y;
                island_x = registry.motions.get(e2).position.x;
                island_y = registry.motions.get(e2).position.y;
                /*std::cout << "SHIP ISLAND COLLISION WITH SHIP AT " << ship_x << ", " << ship_y << " AND ISLAND AT "*/
                /*<< island_x << ", " << island_x << std::endl;*/
            } else {  // e2 is the ship
                ship_x = registry.motions.get(e2).position.x;
                ship_y = registry.motions.get(e2).position.y;
                island_x = registry.motions.get(e1).position.x;
                island_y = registry.motions.get(e1).position.y;
                /*std::cout << "SHIP ISLAND COLLISION WITH SHIP AT " << ship_x << ", " << ship_y << " AND ISLAND AT "*/
                /*<< island_x << ", " << island_x << std::endl;*/
            }
            // CameraSystem::GetInstance()->inverse_velocity(ship_x, ship_y, island_x, island_y);
            CameraSystem::GetInstance()->inverse_velocity();
        }
    }

    for (Entity entity : collisions_to_remove) {
        if (registry.collisions.has(entity)) {
            registry.collisions.remove(entity);
        }
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
    return bool(glfwWindowShouldClose(window));
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

    Scene* scene = SceneManager::getInstance().getCurrentScene();
    if (scene) {
        scene->HandleInput(key, action, mod);
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    // record the current mouse position
    mouse_pos_x = mouse_position.x;
    mouse_pos_y = mouse_position.y;
    Scene* scene = SceneManager::getInstance().getCurrentScene();
    if (scene) {
        scene->HandleMouseMove(mouse_position);
    }
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
    // on button press
    Scene* scene = SceneManager::getInstance().getCurrentScene();
    if (scene) {
        scene->HandleMouseClick(button, action, mods);
    }
}
