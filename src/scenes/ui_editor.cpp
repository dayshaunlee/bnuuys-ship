#include "scenes/ui_editor.hpp"
#include <glm/ext/vector_float2.hpp>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "bnuui/buttons.hpp"

EditorUI::EditorUI() {
    this->name = "Editor";
}

void EditorUI::Init() {
    InitializeUI();
}

float val = 100;
vec2 mouse_pos;
void EditorUI::InitializeUI() {
    auto player_box = std::make_shared<bnuui::Box>(vec2(96, 96), vec2(96, 96), 0.0f);
    auto player_status = std::make_shared<bnuui::PlayerStatus>(vec2(96, 96), vec2(60, 60), 0.0f, val, val);
    auto slider_bg = std::make_shared<bnuui::LongBox>(vec2(256, 96), vec2(240, 72), 0.0f);

    scene_ui.insert(slider_bg);
    scene_ui.insert(player_box);
    scene_ui.insert(player_status);

    for (auto elem : scene_ui.getElems()) {
        elem->setOnActive([](bnuui::Element& e) { e.position = mouse_pos; });

        elem->setOnClick([](bnuui::Element& e) {
            std::cout << "POSITION" << std::endl;
            std::cout << e.position.x << " " << e.position.y << std::endl;
        });
    }
}

void EditorUI::Exit() {
    // Delete all components and stuff from this scene.
}

void EditorUI::HandleInput(int key, int action, int mod) {
    if (action == GLFW_RELEASE && key == GLFW_KEY_1) {
        auto player_box = std::make_shared<bnuui::Box>(mouse_pos, vec2(96, 96), 0.0f);
        player_box->setOnActive([](bnuui::Element& e) { e.position = mouse_pos; });

        player_box->setOnClick([](bnuui::Element& e) {
            std::cout << "POSITION" << std::endl;
            std::cout << e.position.x << " " << e.position.y << std::endl;
        });
        scene_ui.insert(player_box);
    }
}

void EditorUI::HandleMouseMove(vec2 pos) {
    mouse_pos = pos;
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

void EditorUI::HandleMouseClick(int button, int action, int mods) {
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

void EditorUI::Update(float dt) {
    scene_ui.update(dt);
}
