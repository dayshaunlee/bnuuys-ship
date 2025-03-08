#include "sceneManager/scene.hpp"
#include "sceneManager/scene_manager.hpp"
#include "scenes/level_01.hpp"
#include "scenes/main_menu.hpp"
#include "scenes/ui_editor.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "render_system.hpp"
#include "world_system.hpp"
#include "animation_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main() {
    // global systems
    WorldSystem world_system;
    RenderSystem renderer_system;
    AnimationSystem animation_system;

    // initialize window
    GLFWwindow* window = world_system.create_window();

    if (!window) {
        // Time to read the error message
        std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
        getchar();
        return EXIT_FAILURE;
    }

    if (!world_system.start_and_load_sounds()) std::cerr << "ERROR: Failed to start or load sounds." << std::endl;

    // initialize the main systems
    renderer_system.init(window);
    world_system.init(&renderer_system);

    // variable timestep loop
    auto t = Clock::now();

    SceneManager& scene_manager = SceneManager::getInstance();

    Scene* mm = new MainMenuScene();
    Scene* l1 = new Level01(&world_system);
    Scene* ui_editor = new EditorUI();

    scene_manager.registerScene(mm);
    scene_manager.registerScene(l1);
    scene_manager.registerScene(ui_editor);

    scene_manager.switchScene("Main Menu");

    while (!world_system.is_over()) {
        glfwPollEvents();

        auto now = Clock::now();
        float elapsed_ms = (float) (std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
        t = now;
        scene_manager.checkSceneSwitch();

        Scene* s = scene_manager.getCurrentScene();
        if (s != nullptr) s->Update(elapsed_ms);

        renderer_system.draw();
    }

    delete (l1);
    delete (mm);
    delete (ui_editor);

    return EXIT_SUCCESS;
}
