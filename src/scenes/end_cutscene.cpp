#include "scenes/end_cutscene.hpp"
#include <glm/ext/matrix_transform.hpp>
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

EndCutscene::EndCutscene() {
    this->name = "EndCutscene";
}

void EndCutscene::Init() {
    cutscene_image = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 + 40), vec2(450, 450), 0.0f);
    auto black_bg = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2), vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX), 0.0f);
    black_bg->color = vec3(0,0,0);
    black_bg->texture = TEXTURE_ASSET_ID::MAIN_MENU_BG; 

    credit_text_image = std::make_shared<bnuui::Box>(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 + 40), vec2(450, 450), 0.0f);

    scene_ui.insert(black_bg);
    scene_ui.insert(cutscene_image);
    scene_ui.insert(credit_text_image);

    Sound bg_music;
    bg_music.sound_type = SOUND_ASSET_ID::END_MUSIC;
    bg_music.volume = 15;
    registry.sounds.insert(Entity(), bg_music);
}

void EndCutscene::Exit() {
    Mix_HaltChannel(-1);
}

void EndCutscene::HandleInput(int key, int action, int mod) {
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        SceneManager::getInstance().switchScene("Main Menu");
    }
}

void EndCutscene::HandleMouseMove(vec2 pos) {
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

void EndCutscene::HandleMouseClick(int button, int action, int mods) {
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

void EndCutscene::checkDialogs(float dt) { 
    if (phase == 0) {
        cutscene_image->texture = TEXTURE_ASSET_ID::END_CUTSCENE_HUG;
        cutscene_image->position = vec2(200, WINDOW_HEIGHT_PX/2);
        cutscene_image->scale = vec2(300, 500);
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_M1;
        credit_text_image->position = vec2(WINDOW_WIDTH_PX/2 + 200, WINDOW_HEIGHT_PX/2);
        credit_text_image->scale = vec2{400, 400};
    } else if (phase == 1) {
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_M2;
        credit_text_image->position = vec2(WINDOW_WIDTH_PX/2 + 150, WINDOW_HEIGHT_PX/2);
    } else if (phase == 2) {
        cutscene_image->texture = TEXTURE_ASSET_ID::END_CUTSCENE_SAIL;
        cutscene_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/4 + 50.f);
        cutscene_image->scale = vec2(700, 250);
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_M3;
        credit_text_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 200.f);
        credit_text_image->scale = vec2{800, 400};
    } else if (phase == 3) {
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_M4;
    } else if (phase == 4) {
        cutscene_image->texture = TEXTURE_ASSET_ID::END_CUTSCENE_VILLAGE;
        cutscene_image->scale = vec2(2*WINDOW_WIDTH_PX, 1.75*WINDOW_HEIGHT_PX);
        credit_text_image->scale = vec2{400,400};
        credit_text_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2);
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_ASSETS;
    } else if (phase == 5) {
        credit_text_image->texture = TEXTURE_ASSET_ID::TEXT_THANKS;
        credit_text_image->position = vec2(WINDOW_WIDTH_PX/2-20, WINDOW_HEIGHT_PX/2);
    }
    if (dialogue_timer_ms > 0) {
        dialogue_timer_ms -= dt;
    } else {
        phase++;
        if (phase >= 6) {
            SceneManager::getInstance().switchScene("Main Menu");
            return;
        }
        dialogue_timer_ms = DIALOGUE_TIME_MS;
        if (phase == 4) {
            cutscene_image->position = vec2(WINDOW_WIDTH_PX/2, 100);
        } else if (phase == 5) {
            dialogue_timer_ms = 10000;
        }
    }
}


void EndCutscene::Update(float dt) {
    scene_ui.update(dt);
    checkDialogs(dt);
    if (phase >= 6) {
        SceneManager::getInstance().switchScene("Main Menu"); 
    }
    if (phase >= 4) {
        cutscene_image->position += vec2(0, 0.1f*dt);
        if (cutscene_image->position.y >= WINDOW_HEIGHT_PX-100.f) {
            cutscene_image->position = vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX-100.f);
        }
    }
}
