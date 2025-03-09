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
    fpsCounter = 0;
}

WorldSystem::~WorldSystem() {
    // Destroy music components
    if (background_music != nullptr) Mix_FreeMusic(background_music);
    if (enemy_incoming != nullptr) Mix_FreeMusic(enemy_incoming);
    if (island_ship_collision != nullptr) Mix_FreeChunk(island_ship_collision);
    if (enemy_ship_collision != nullptr) Mix_FreeChunk(enemy_ship_collision);
    //if (projectile_shoot != nullptr) Mix_FreeChunk(projectile_shoot);
    if (projectile_jail_collision != nullptr) Mix_FreeChunk(projectile_jail_collision);
    if (projectile_enemy_collision != nullptr) Mix_FreeChunk(projectile_enemy_collision);
    if (game_over != nullptr) Mix_FreeChunk(game_over);
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
    std::string fullTitle = "Bnuuy's Ship      FPS: " + std::to_string(fpsCounter);    

    window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, fullTitle.c_str(), nullptr, nullptr);
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
    background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
    enemy_incoming = Mix_LoadMUS(audio_path("enemy_incoming.wav").c_str());
    island_ship_collision = Mix_LoadWAV(audio_path("island-ship_collision.wav").c_str());
    enemy_ship_collision = Mix_LoadWAV(audio_path("ship-enemy_collision.wav").c_str());
    projectile_enemy_collision = Mix_LoadWAV(audio_path("projectile-enemy_collision.wav").c_str());
    projectile_jail_collision = Mix_LoadWAV(audio_path("projectile-jail_collision.wav").c_str());
    game_over = Mix_LoadWAV(audio_path("game_over.wav").c_str());

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    if (background_music == nullptr || enemy_incoming == nullptr ||
        island_ship_collision == nullptr || enemy_ship_collision == nullptr || projectile_enemy_collision == nullptr) {
        fprintf(stderr,
                "Failed to load sounds\n %s\n %s\n %s\n make sure the data "
                "directory is present",
                audio_path("music.wav").c_str(),
                audio_path("enemy_incoming.wav").c_str(),
                audio_path("island-ship_collision.wav").c_str(),
                audio_path("ship-enemy_collision.wav").c_str(),
                audio_path("projectile-enemy_collision.wav").c_str(),
                audio_path("projectile-jail_collision.wav").c_str(),
                audio_path("game_over.wav").c_str());
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
    std::string fullTitle = "Bnuuy's Ship      FPS: " + std::to_string(fpsCounter);    
    glfwSetWindowTitle(window, fullTitle.c_str());

    assert(registry.screenStates.components.size() <= 1);
    ScreenState& screen = registry.screenStates.components[0];

    // TODO: Change the music
    /*if (!CameraSystem::GetInstance()->IsEnemyOnScreen()) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
        Mix_PlayMusic(enemy_incoming, -1);
    }*/

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
        Entity e1 = collision_container.entities[i];
        Entity e2 = collision_container.components[i].other;
        if (!registry.motions.has(e1) || !registry.motions.has(e2)) {
            collisions_to_remove.push_back(e1);
            collisions_to_remove.push_back(e2);
            continue;
        }

        // Projectile - Enemy collision
        if (registry.projectiles.has(e1) && registry.enemies.has(e2)) {
            Projectile& projectile = registry.projectiles.get(e1);
            Enemy& enemy = registry.enemies.get(e2);

            enemy.health -= projectile.damage;

            if (enemy.health <= 0) registry.remove_all_components_of(e2);
            registry.remove_all_components_of(e1);
            // Play sound
            Mix_PlayChannel(-1, projectile_enemy_collision, 0);

        } else if (registry.projectiles.has(e2) && registry.enemies.has(e1)) {
            Projectile& projectile = registry.projectiles.get(e2);
            Enemy& enemy = registry.enemies.get(e1);

            enemy.health -= projectile.damage;
            if (enemy.health <= 0) registry.remove_all_components_of(e1);

            registry.remove_all_components_of(e2);
            // Play sound
            Mix_PlayChannel(-1, projectile_enemy_collision, 0);
        }

        // Projectile - Bunny collision
        if (registry.projectiles.has(e1) && registry.bunnies.has(e2) && registry.bunnies.get(e2).is_jailed) {
            Projectile& projectile = registry.projectiles.get(e1);
            Bunny& bunny = registry.bunnies.get(e2);

            bunny.jail_health -= projectile.damage;

            if (bunny.jail_health <= 0) {
                registry.renderRequests.get(e2).used_texture = TEXTURE_ASSET_ID::BUNNY_NOT_JAILED;
                // Play sound
                Mix_PlayChannel(-1, projectile_jail_collision, 0);
                bunny.is_jailed = false;
            }
            registry.remove_all_components_of(e1);

        } else if (registry.projectiles.has(e2) && registry.bunnies.has(e1) && registry.bunnies.get(e1).is_jailed) {
            Projectile& projectile = registry.projectiles.get(e2);
            Bunny& bunny = registry.bunnies.get(e1);

            bunny.jail_health -= projectile.damage;

            if (bunny.jail_health <= 0) {
                registry.renderRequests.get(e1).used_texture = TEXTURE_ASSET_ID::BUNNY_NOT_JAILED;
                // Play sound
                Mix_PlayChannel(-1, projectile_jail_collision, 0);
                bunny.is_jailed = false;
            }
            registry.remove_all_components_of(e2);
        }

        // Enemy - Ship collision
        if (registry.enemies.has(e1) && registry.ships.has(e2)) {
            registry.ships.get(e2).health -= 50.0f;
            registry.remove_all_components_of(e1);
            // Play sound
            Mix_PlayChannel(-1, enemy_ship_collision, 0);

            // When Player dies (ship health is <= 0)
            if(registry.ships.get(e2).health <= 0.0f){
                handle_player_death();
                return;
            }
            continue;
        } else if (registry.enemies.has(e2) && registry.ships.has(e1)) {
            registry.ships.get(e1).health -= 50.0f;
            registry.remove_all_components_of(e2);
            // Play sound
            Mix_PlayChannel(-1, enemy_ship_collision, 0);

            // When Player dies (ship health is <= 0)
            if(registry.ships.get(e1).health <= 0.0f){
                handle_player_death();
                return;
            }
            continue;
        }



        // Ship - Island collision
        if ((registry.ships.has(e1) && registry.islands.has(e2)) ||
            (registry.ships.has(e2) && registry.islands.has(e1))) {
            collisions_to_remove.push_back(e1);
            collisions_to_remove.push_back(e2);
            // Play sound
            Mix_PlayChannel(-1, island_ship_collision, 0);
            CameraSystem::GetInstance()->setToPreviousPosition();
        }

        // Ship - Base collision
        if ((registry.ships.has(e1) && registry.base.has(e2)) ||
            (registry.ships.has(e2) && registry.base.has(e1))) {
            collisions_to_remove.push_back(e1);
            collisions_to_remove.push_back(e2);
            // just print debug stuff rn, behaviour is handled in different system
            //std::cout << "island over base" << std::endl;            
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

void WorldSystem::change_title(std::string title) {
    glfwSetWindowTitle(window, title.c_str());
}

void WorldSystem::handle_player_death(){
    SceneManager& sceneManager = SceneManager::getInstance();
    std::cout << "Switching to player death scene..." << std::endl;
    std::string currentSceneName = sceneManager.getCurrentScene()->getName();
    sceneManager.setRestartScence(currentSceneName);
    sceneManager.switchScene("Death Scene");
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // exit game w/ ESC
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        close_window();
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
        // Play sound
        scene->HandleMouseClick(button, action, mods);
    }
}