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
#include "saveload_system.hpp"
#include <filesystem>

namespace fs = std::filesystem;

MainMenuScene::MainMenuScene() {
    this->name = "Main Menu";
}

void MainMenuScene::Init() {
    // Create the UI Title Screen.
    auto play_btn = std::make_shared<bnuui::PlayButton>(vec2((WINDOW_WIDTH_PX/2) - 180, 0.8*WINDOW_HEIGHT_PX), vec2(300, 100), 0.0f);
    auto tutorial_btn = std::make_shared<bnuui::TutorialButton>(vec2((WINDOW_WIDTH_PX/2) + 180, 0.8*WINDOW_HEIGHT_PX), vec2(300, 100), 0.0f);
    auto exit_btn = std::make_shared<bnuui::ExitButton>(vec2((WINDOW_WIDTH_PX/2) + 180, 0.8*WINDOW_HEIGHT_PX + 90), vec2(300, 100), 0.0f);
    auto bg = std::make_shared<bnuui::Box>(vec2((WINDOW_WIDTH_PX/2), WINDOW_HEIGHT_PX/2), vec2(WINDOW_WIDTH_PX*1.25f, WINDOW_HEIGHT_PX), 0.0f);

    bg->texture = TEXTURE_ASSET_ID::MAIN_MENU_BG;

    tutorial_btn->setOnClick([](bnuui::Element& e) {
        SceneManager::getInstance().switchScene("Tutorial Level");
    });

    play_btn->setOnClick([](bnuui::Element& e) {
        SceneManager::getInstance().switchScene("Level 1");
    });

    exit_btn->setOnClick([](bnuui::Element& e) {
        // todo here
    });

    scene_ui.insert(bg);
    scene_ui.insert(play_btn);
    scene_ui.insert(tutorial_btn);
    scene_ui.insert(exit_btn);
    // scene_ui.insert(txt);

    if (!std::filesystem::is_empty(data_path() + "/level_save.json")) {
        auto continue_btn = std::make_shared<bnuui::ContinueButton>(vec2((WINDOW_WIDTH_PX/2) - 180, 0.8*WINDOW_HEIGHT_PX + 90), vec2(300, 100), 0.0f);
        continue_btn->setOnClick([](bnuui::Element& e) {
            SaveLoadSystem& saveLoadSystem = SaveLoadSystem::getInstance();
            GameData gameData;
            saveLoadSystem.loadGame(gameData, "level_save.json");
            std::cout << "game loaded" << std::endl;
            saveLoadSystem.loadedGameData = gameData;
            saveLoadSystem.hasLoadedData = true;
    
            SceneManager::getInstance().switchScene(gameData.levelName);
        });
        scene_ui.insert(continue_btn);
    }

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
