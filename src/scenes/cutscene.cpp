#include "scenes/cutscene.hpp"
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include "GLFW/glfw3.h"
#include "bnuui/bnuui.hpp"
#include "bnuui/buttons.hpp"
#include "common.hpp"
#include "sceneManager/scene_manager.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

IntroCutscene::IntroCutscene() {
    this->name = "IntroCutscene";
}

void IntroCutscene::Init() {
    cutscene_image = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 + 40), vec2(450, 450), 0.0f);
    auto black_bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2), vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), 0.0f);
    black_bg->color = vec3(0,0,0);
    black_bg->texture = TEXTURE_ASSET_ID::MAIN_MENU_BG; 

    dialog = std::make_shared<bnuui::TextLabel>(vec2(0,100), 1.5f, " ");
    rendered_dialog_text = " ";
    dialog->color = vec3(1,1,1);
    cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_1;
    cutscene_image->color = vec3(0,0,0);

    dialog->setOnUpdate([&](bnuui::Element& e, float dt) {
        static_cast<bnuui::TextLabel&>(e).setText(rendered_dialog_text);
    });
 
    scene_ui.insert(black_bg);
    scene_ui.insert(cutscene_image);
    scene_ui.insert(dialog);

    Sound bg_music;
    bg_music.sound_type = SOUND_ASSET_ID::CUTSCENE_MUSIC;
    registry.sounds.insert(Entity(), bg_music);

    curr_line = dialog_parts[0];
}

void IntroCutscene::Exit() {

}

void IntroCutscene::HandleInput(int key, int action, int mod) {
}

void IntroCutscene::HandleMouseMove(vec2 pos) {
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

void IntroCutscene::HandleMouseClick(int button, int action, int mods) {
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

void IntroCutscene::revealCharacters(float dt) {
    if (char_index < curr_line.size()) {
        static float char_timer = 0;
        char_timer += dt;

        if (char_timer >= CHAR_TIME_MS) {
            char_timer = 0;
            rendered_dialog_text += curr_line[char_index++];
        }
    }
}

void IntroCutscene::checkDialogs(float dt) {
    if (phase == 13) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 150.0f, WINDOW_HEIGHT_PX/2);
        cutscene_image->visible = false;
    } else if (phase == 0) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 250.0f, WINDOW_HEIGHT_PX/2);
        cutscene_image->visible = false;
    } else {
        dialog->position = vec2(100, 100);
        cutscene_image->visible = true;
    }

    if (dialogue_timer_ms > 0) {
        dialogue_timer_ms -= dt;
    } else {
        phase++;
        curr_line = dialog_parts[phase];
        rendered_dialog_text = " ";
        char_index = 0;
        if (phase >= 3 && phase <= 5) {
            cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_2;
            dialogue_timer_ms = PHASE1_TIME;
        } else if (phase >= 6 && phase <= 9) {
            cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_3;
            dialogue_timer_ms = PHASE2_TIME;
        } else if (phase >= 10) {
            cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_4;
            cutscene_image->scale = vec2(450, 650);
            dialogue_timer_ms = PHASE3_TIME;
            cutscene_image->visible = true;
        } else {
            dialogue_timer_ms = PHASE0_TIME;
        }
        if (phase == 1 || phase == 3 || phase == 6 || phase == 10) {
            cutscene_image->color = vec3(0,0,0);
        }
    }
}

void IntroCutscene::undarkenImage(float dt) {
    static float dark_timer = 0;
    dark_timer += dt;

    if (dark_timer >= CHAR_TIME_MS) {
        dark_timer = 0;
        cutscene_image->color += (vec3(1,1,1) - cutscene_image->color) * 0.0025f * dt; 
        cutscene_image->color = glm::clamp(cutscene_image->color, vec3(0,0,0), vec3(1,1,1));
    }
}

void IntroCutscene::Update(float dt) {
    scene_ui.update(dt);
    checkDialogs(dt);
    revealCharacters(dt);
    undarkenImage(dt);
}
