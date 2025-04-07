#pragma once

#include <memory>
#include "bnuui/buttons.hpp"
#include "sceneManager/scene.hpp"
class IntroCutscene : public Scene {
public:
    IntroCutscene();
    void Init() override;
    void Exit() override;
    void HandleInput(int key, int action, int mod) override;
    void HandleMouseMove(glm::vec2 mousePos) override;
    void HandleMouseClick(int button, int action, int mods) override;
    void Update(float dt) override;
private:
    std::shared_ptr<bnuui::Box> cutscene_image;
    std::shared_ptr<bnuui::TextLabel> dialog;
    std::shared_ptr<bnuui::TextLabel> disclaimer;
    std::vector<std::string> dialog_parts = {
        // Phase 1 [0-2] ~20 seconds.
        "A long time ago in an Island Far Far Away...",
        "A village of bunnies lived in peace.",
        "Their island, a paradise under the moonlit night.",

        // Phase 2 [3-5] ~20 seconds
        "But peace never lasts...",
        "A bunch of cows and chickens came in boats.",
        "Together, they banded and invaded the bunnies!",

        // Phase 3 [6-9] ~30 seconds
        "One by one, the bunnies were taken.",
        "They were caged and left to await a terrible fate...",
        "All left alone.",
        "The bunnies yearn for freedom.",

        // Phase 3 [10-13] ~ 23 seconds
        "But one bunny escaped.",
        "One bunny refuses to give up.",
        "Armed with only a raft and a dream...",
        "The great rescue begins.",
    };

    std::string rendered_dialog_text;
    std::string curr_line;

    int dialog_index = 0;
    size_t char_index = 0;

    const float DIALOGUE_TIME_MS = 5000.f;

    const float PHASE0_TIME = (20.f * 1000.f) / 3.f;
    const float PHASE1_TIME = (18.f * 1000.f) / 3.f;
    const float PHASE2_TIME = (30.f * 1000.f) / 4.f;
    const float PHASE3_TIME = (23.f * 1000.f) / 4.f;

    const float CHAR_TIME_MS = 75.0f;

    float dialogue_timer_ms = PHASE0_TIME;
    int phase = 0;

    void checkDialogs(float dt);
    void revealCharacters(float dt);
    void undarkenImage(float dt);
};
