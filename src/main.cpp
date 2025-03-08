#include "bnuui/bnuui.hpp"
#include "sceneManager/scene.hpp"
#include "sceneManager/scene_manager.hpp"
#include "scenes/death_scene.hpp"
#include "scenes/level_01.hpp"
#include "scenes/main_menu.hpp"
#include "scenes/ui_editor.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "animation_system.hpp"
#include "bnuui/bnuui.hpp"
#include "camera_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main() {
    // global systems
    AISystem ai_system;
    WorldSystem world_system;
    RenderSystem renderer_system;
    PhysicsSystem physics_system;
    AnimationSystem animation_system;
    int frameCounter = 0;
    float msCounter = 0;

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
    Scene* death = new DeathScene();

    scene_manager.registerScene(mm);
    scene_manager.registerScene(l1);
    scene_manager.registerScene(ui_editor);
    scene_manager.registerScene(death);

    scene_manager.switchScene("Main Menu");

    while (!world_system.is_over()) {
        glfwPollEvents();

        auto now = Clock::now();
        float elapsed_ms = (float) (std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
        t = now;
        
        msCounter += elapsed_ms;
        // std::cout << "msCounter: " << msCounter << std::endl;
        frameCounter++;
        // std::cout << frameCounter << std::endl;
        if(msCounter >= 1000){
            world_system.fpsCounter = frameCounter* (msCounter/1000.f);
            
            msCounter = 0;
            frameCounter = 0;
        }



        // std::cout << "FPS: " << world_system.fpsCounter << std::endl;

        scene_manager.checkSceneSwitch();

        Scene* s = scene_manager.getCurrentScene();
        if (s != nullptr) s->Update(elapsed_ms);
        world_system.step(elapsed_ms);

        renderer_system.draw();
    }

    delete (l1);
    delete (mm);
    delete (ui_editor);

    return EXIT_SUCCESS;
}
