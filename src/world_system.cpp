// Header
#include "world_system.hpp"
#include "GLFW/glfw3.h"
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

    // debugging for memory/component leaks
    registry.list_all_components();

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
    ComponentContainer<Collision>& collision_container = registry.collisions;
    for (uint i = 0; i < collision_container.components.size(); i++) {
        // Handle collision here.
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
    if (action == GLFW_PRESS) {
        int tile_x = (int) (mouse_pos_x / GRID_CELL_WIDTH_PX);
        int tile_y = (int) (mouse_pos_y / GRID_CELL_HEIGHT_PX);

        std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
        std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
    }
    Scene* scene = SceneManager::getInstance().getCurrentScene();
    if (scene) {
        scene->HandleMouseClick(button, action, mods);
    }
}
