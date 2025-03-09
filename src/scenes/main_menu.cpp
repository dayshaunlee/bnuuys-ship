#include "scenes/main_menu.hpp"
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "bnuui/buttons.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "tinyECS/components.hpp"

MainMenuScene::MainMenuScene() {
    this->name = "Main Menu";
}

void MainMenuScene::Init() {
    // Create the UI Title Screen.
    auto play_btn = std::make_shared<bnuui::PlayButton>(vec2(WINDOW_WIDTH_PX/2, 0.8*WINDOW_HEIGHT_PX), vec2(350, 100), 0.0f);
    auto bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(WINDOW_WIDTH_PX*1.25f, WINDOW_HEIGHT_PX), 0.0f);
    bg->texture = TEXTURE_ASSET_ID::MAIN_MENU_BG;
    play_btn->setOnClick([](bnuui::Element& e) {
        SceneManager::getInstance().switchScene("Tutorial Level");
    });

    scene_ui.insert(bg);
    scene_ui.insert(play_btn);
}

void MainMenuScene::Exit() {
}

void MainMenuScene::HandleInput(int key, int action, int mod) {
}

void MainMenuScene::HandleMouseMove(vec2 pos) {
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

void MainMenuScene::HandleMouseClick(int button, int action, int mods) {
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

void MainMenuScene::Update(float dt) {
    scene_ui.update(dt);
}
