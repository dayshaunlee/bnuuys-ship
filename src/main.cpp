#include "camera_system.hpp"
#include "common.hpp"
#include "sceneManager/scene.hpp"
#include "sceneManager/scene_manager.hpp"
#include "scenes/cutscene.hpp"
#include "scenes/death_scene.hpp"
#include "scenes/end_cutscene.hpp"
#include "scenes/next_level_scene.hpp"
#include "scenes/victory_scene.hpp"
#include "scenes/level_01.hpp"
#include "scenes/level_02.hpp"
#include "scenes/level_03.hpp"
#include "scenes/level_04.hpp"
#include "scenes/main_menu.hpp"
#include "scenes/tutorial.hpp"
#include "tinyECS/components.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "render_system.hpp"
#include "world_system.hpp"
#include "animation_system.hpp"
#include "sound_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main() {
    // global systems
    WorldSystem world_system;
    RenderSystem renderer_system;
    SoundSystem sound_system;
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

    //if (!world_system.start_and_load_sounds()) std::cerr << "ERROR: Failed to start or load sounds." << std::endl;

    // initialize the main systems
    renderer_system.init(window);
    renderer_system.fontInit(font_path("sproutslandfont.ttf"), 16);
    world_system.init(&renderer_system);
    sound_system.init();

    // variable timestep loop
    auto t = Clock::now();

    SceneManager& scene_manager = SceneManager::getInstance();

    Scene* mm = new MainMenuScene();
    Scene* tutorial = new TutorialLevel(&world_system, "m4_tutorial.json", TEXTURE_ASSET_ID::TUTORIAL_BACKGROUND);
    Scene* l1 = new Level01(&world_system, "m3_level1.json", TEXTURE_ASSET_ID::LEVEL01_BACKGROUND);
    Scene* l2 = new Level02(&world_system, "m3_level2.json", TEXTURE_ASSET_ID::LEVEL02_BACKGROUND);
    Scene* l3 = new Level03(&world_system, "m3_level3.json", TEXTURE_ASSET_ID::LEVEL03_BACKGROUND);
    Scene* l4 = new Level04(&world_system, "m3_level4.json", TEXTURE_ASSET_ID::LEVEL04_BACKGROUND);
    Scene* death = new DeathScene();
    Scene* levelTransition = new NextLevelScene();
    Scene* cutscene = new IntroCutscene();
    Scene* victory = new VictoryScene();
    Scene* end_credits = new EndCutscene();

    scene_manager.registerScene(mm);
    scene_manager.registerScene(death);
    scene_manager.registerScene(levelTransition);
    scene_manager.registerScene(victory);

    scene_manager.registerScene(tutorial);
    scene_manager.registerScene(l1);
    scene_manager.registerScene(l2);
    scene_manager.registerScene(l3);
    scene_manager.registerScene(l4);

    scene_manager.registerScene(cutscene);
    scene_manager.registerScene(end_credits);

    scene_manager.switchScene("IntroCutscene");

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
        sound_system.play();
        
    }

    delete mm;
    delete tutorial;
    delete l1;
    delete l2;
    delete l3;
    delete l4;
    delete death;
    delete levelTransition;
    delete cutscene;
    delete victory;
    delete end_credits;
    delete (CameraSystem::GetInstance());

    return EXIT_SUCCESS;
}
