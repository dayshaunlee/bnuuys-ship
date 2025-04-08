#pragma once

#include <memory>
#include "bnuui/buttons.hpp"
#include "sceneManager/scene.hpp"
class EndCutscene : public Scene {
public:
    EndCutscene();
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override;
private:
    std::shared_ptr<bnuui::Box> cutscene_image;
    std::shared_ptr<bnuui::Box> credit_text_image;

    const float DIALOGUE_TIME_MS = 5000.f;

    float dialogue_timer_ms = DIALOGUE_TIME_MS;
    int phase = 0;

    void checkDialogs(float dt);
};
