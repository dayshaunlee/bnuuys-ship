#include "scenes/victory_scene.hpp"
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


VictoryScene::VictoryScene() {
    this->name = "Victory Scene";
}

void VictoryScene::Init() {
    // Create the UI Title Screen.
    auto continue_btn = std::make_shared<bnuui::ContinueButton>(vec2(WINDOW_WIDTH_PX/2, 0.65*WINDOW_HEIGHT_PX), vec2(350, 100), 0.0f);
    auto bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2 - 25.f, WINDOW_HEIGHT_PX/2), vec2(WINDOW_WIDTH_PX*1.25f, WINDOW_HEIGHT_PX), 0.0f);
    bg->texture = TEXTURE_ASSET_ID::VICTORY_BG;
    continue_btn->setOnClick([](bnuui::Element& e) {
        SceneManager::getInstance().switchScene("Main Menu");
    });

    scene_ui.insert(bg);
    scene_ui.insert(continue_btn);
}

void VictoryScene::Exit() {
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

void VictoryScene::HandleInput(int key, int action, int mod) {
}

void VictoryScene::HandleMouseMove(vec2 pos) {
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

void VictoryScene::HandleMouseClick(int button, int action, int mods) {
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
}

void VictoryScene::Update(float dt) {
    scene_ui.update(dt);
}
