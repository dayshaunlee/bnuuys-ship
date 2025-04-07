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
#include <world_system.hpp>

IntroCutscene::IntroCutscene() {
    this->name = "IntroCutscene";
}

void IntroCutscene::Init() {
    cutscene_image = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 + 40), vec2(450, 450), 0.0f);
    auto black_bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2), vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), 0.0f);
    black_bg->color = vec3(0,0,0);
    black_bg->texture = TEXTURE_ASSET_ID::MAIN_MENU_BG; 

    dialog = std::make_shared<bnuui::TextLabel>(vec2(0,100), 1.5f, " ");

    disclaimer = std::make_shared<bnuui::TextLabel>(vec2(0,100), 1, "Press (Space) to skip cutscene");
    disclaimer->color = vec3(1,1,1);

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
    scene_ui.insert(disclaimer);

    Sound bg_music;
    bg_music.sound_type = SOUND_ASSET_ID::CUTSCENE_MUSIC;
    bg_music.volume = 100;
    registry.sounds.insert(Entity(), bg_music);

    curr_line = dialog_parts[0];
}

void IntroCutscene::Exit() {
    Mix_HaltChannel(-1);
}

void IntroCutscene::HandleInput(int key, int action, int mod) {
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        SceneManager::getInstance().switchScene("Main Menu");
    }
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
    if (phase == 0) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 250.0f, WINDOW_HEIGHT_PX/2);
        disclaimer->position = vec2((float) WINDOW_WIDTH_PX/2 - 125.0f, WINDOW_HEIGHT_PX-10);
        cutscene_image->visible = false;
    } else if (phase == 1) {
        cutscene_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2);
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX+175.0f);
        dialog->position = vec2(200, WINDOW_HEIGHT_PX-25.0f);
        cutscene_image->visible = true;
        disclaimer->visible = false;
        disclaimer->setText(" ");
    } else if (phase == 2) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_BUNNY_VILLAGE;
        dialog->position = vec2(100, WINDOW_HEIGHT_PX-25.0f);
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
    } else if (phase == 3) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 150.0f, WINDOW_HEIGHT_PX/2);
        cutscene_image->visible = false;
    } else if (phase == 4) {
        cutscene_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2);
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX+175.0f);
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_2;
        dialog->position = vec2(150, WINDOW_HEIGHT_PX-25.0f);
        cutscene_image->visible = true;
    } else if (phase == 5) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_COW_CAPTURE;
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
    } else if (phase == 6) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_CRYING_BUNNY;
        dialog->position = vec2(200, WINDOW_HEIGHT_PX-25.0f);
    } else if (phase == 7) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_MANY_BUNNY_CAGED;
        dialog->position = vec2(100, WINDOW_HEIGHT_PX-25.0f);
    } else if (phase == 8) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 100.0f, WINDOW_HEIGHT_PX/2);
        cutscene_image->visible = false;
    } else if (phase == 9) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_3;
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX+185.0f);
        cutscene_image->visible = true;
        dialog->position = vec2(225, WINDOW_HEIGHT_PX-25.0f);
    } else if (phase == 10) {
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_BUNNY_ESCAPE;
        dialog->position = vec2(225, WINDOW_HEIGHT_PX-25.0f);
    } else if (phase == 11) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_BUNNY_BUILD;
    } else if (phase == 12) {
        cutscene_image->texture = TEXTURE_ASSET_ID::CUTSCENE_4;
        cutscene_image->scale = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX+500.0f);
    } else if (phase == 13) {
        dialog->position = vec2((float) WINDOW_WIDTH_PX/2 - 150.0f, WINDOW_HEIGHT_PX/2);
        cutscene_image->visible = false;
    }
    else {
        dialog->position = vec2(100, WINDOW_HEIGHT_PX-25.0f);
        cutscene_image->visible = true;
    }

    if (dialogue_timer_ms > 0) {
        dialogue_timer_ms -= dt;
    } else {
        phase++;
        if (phase >= dialog_parts.size()) {
            SceneManager::getInstance().switchScene("Main Menu");
            return;
        }
        curr_line = dialog_parts[phase];
        rendered_dialog_text = " ";
        char_index = 0;
        if (phase >= 3 && phase <= 5) {
            dialogue_timer_ms = PHASE1_TIME;
        } else if (phase >= 6 && phase <= 9) {
            dialogue_timer_ms = PHASE2_TIME;
        } else if (phase >= 10) {
            dialogue_timer_ms = PHASE3_TIME;
        } else {
            dialogue_timer_ms = PHASE0_TIME;
        }
        cutscene_image->color = vec3(0,0,0);
    }
}

void IntroCutscene::undarkenImage(float dt) {
    static float dark_timer = 0;
    dark_timer += dt;

    if (dark_timer >= CHAR_TIME_MS/2) {
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

    if (phase >= dialog_parts.size()) {
        SceneManager::getInstance().switchScene("Main Menu"); 
    }
}
