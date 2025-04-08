#include "scenes/next_level_scene.hpp"
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "bnuui/buttons.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "camera_system.hpp"

NextLevelScene::NextLevelScene() {
    this->name = "Next Level Scene";
}

void NextLevelScene::Init() {
    // Create the UI Title Screen.
    auto play_btn = std::make_shared<bnuui::PlayButton>(vec2(WINDOW_WIDTH_PX/2, 0.65*WINDOW_HEIGHT_PX), vec2(350, 100), 0.0f);

    auto bg_water = std::make_shared<bnuui::Box>(
        vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2), vec2(WINDOW_WIDTH_PX * 1.25f, WINDOW_HEIGHT_PX), 0.0f);
    bg_water->texture = TEXTURE_ASSET_ID::WATER_BACKGROUND;

    auto bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(WINDOW_WIDTH_PX*2, WINDOW_HEIGHT_PX), 0.0f);
    std::string nextLevelName = SceneManager::getInstance().getNewLevelSceneName();
    auto text = std::make_shared<bnuui::TextLabel>(vec2(WINDOW_WIDTH_PX/2-122.5f, WINDOW_HEIGHT_PX/2), 5.0f, nextLevelName, true);
    text->color = vec3{115.f/255.f, 75.f/255.f, 50.f/255.f};

    bg->setOnUpdate([nextLevelName](bnuui::Element& e, float dt) {
        std::cout << "next level: " << nextLevelName << std::endl;
        if (nextLevelName == "Level 1") {
            e.texture = TEXTURE_ASSET_ID::LEVEL01_BACKGROUND;
            e.scale = vec2(36.f / 30.f * WINDOW_HEIGHT_PX, WINDOW_HEIGHT_PX);
        } else if (nextLevelName == "Level 2") {
            e.texture = TEXTURE_ASSET_ID::LEVEL02_BACKGROUND;
            e.scale = vec2(50.f / 30.f * WINDOW_HEIGHT_PX, WINDOW_HEIGHT_PX);
        } else if (nextLevelName == "Level 3") {
            e.texture = TEXTURE_ASSET_ID::LEVEL03_BACKGROUND;
            e.scale = vec2(28.f / 60.f * WINDOW_HEIGHT_PX, WINDOW_HEIGHT_PX);
        } else if (nextLevelName == "Level 4") {
            e.texture = TEXTURE_ASSET_ID::LEVEL04_BACKGROUND;
            e.scale = vec2(44.f / 44.f * WINDOW_HEIGHT_PX, WINDOW_HEIGHT_PX);
        }
        else {
            e.texture = TEXTURE_ASSET_ID::NEXT_LEVEL_BG;
            vec2(WINDOW_WIDTH_PX * 1.25f, WINDOW_HEIGHT_PX);
        }
    });
    play_btn->setOnClick([](bnuui::Element& e) {
        std::string nextLevelName = SceneManager::getInstance().getNewLevelSceneName();
        SceneManager::getInstance().switchScene(nextLevelName);
    });

    scene_ui.insert(bg_water);
    scene_ui.insert(bg);
    scene_ui.insert(play_btn);
    scene_ui.insert(text);
}

void NextLevelScene::Exit() {
    scene_ui.clear();
    CameraSystem* cs = CameraSystem::GetInstance();
    cs->position = {0.0f, 0.0f};
    cs->prev_pos = {0.0f, 0.0f};
    cs->vel = {0.0f, 0.0f};
    // Delete all components and stuff from this scene.
    // registry.clear_all_components();

    while (registry.cameras.entities.size() > 0){
	    registry.remove_all_components_of(registry.cameras.entities.back());
	}
    while (registry.renderRequests.entities.size() > 0){
	    registry.remove_all_components_of(registry.renderRequests.entities.back());
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

    while (registry.islands.entities.size() > 0){
	    registry.remove_all_components_of(registry.islands.entities.back());
	}


    while (registry.base.entities.size() > 0){
	    registry.remove_all_components_of(registry.base.entities.back());
	}

    while (registry.bunnies.entities.size() > 0){
	    registry.remove_all_components_of(registry.bunnies.entities.back());
	}
}

void NextLevelScene::HandleInput(int key, int action, int mod) {
}

void NextLevelScene::HandleMouseMove(vec2 pos) {
    // Check if hovering over any UI components.
    std::vector<std::shared_ptr<bnuui::Element>> ui_elems = scene_ui.getElems();
    for (std::shared_ptr<bnuui::Element> ui_elem : ui_elems) {
        if (ui_elem->isPointColliding(pos)) {
            ui_elem->hovering = true;
        } else {
            ui_elem->hovering = false;
        }
    }
}

void NextLevelScene::HandleMouseClick(int button, int action, int mods) {
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
    // Play sound
    Entity sound_entity = Entity();
    Sound& sound = registry.sounds.emplace(sound_entity);
    sound.sound_type = SOUND_ASSET_ID::CLICK;
    sound.volume = 30;
}

void NextLevelScene::Update(float dt) {
    scene_ui.update(dt);
}
