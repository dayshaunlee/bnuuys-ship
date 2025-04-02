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
    std::vector<std::vector<std::string>> dialog_parts = {
        { "A long time ago in an Island Far Far Away...",
          "The bunnies lived in peace.",
          "Their island, a paradise under the moonlit night." },

        { "But peace never lasts...",
          "The Greedy Herd came, hungry for power—and for bunnies." },

        { "One by one, the bunnies were taken.",
          "They were caged and left to await a terrible fate..." },

        { "But one bunny escaped.",
          "One bunny refuses to give up.",
          "Armed with only a raft and a dream...",
          "The great rescue begins." }
    };

    std::string rendered_dialog_text;
    std::string curr_line;

    int dialog_index = 0;
    size_t char_index = 0;

    const float DIALOGUE_TIME_MS = 2000.f;
    const float CHAR_TIME_MS = 50.0f;

    float dialogue_timer_ms = DIALOGUE_TIME_MS;

    enum CutscenePhase {
        PHASE_0,
        PHASE_1,
        PHASE_2,
        PHASE_3,
        PHASE_4,
        PHASE_END
    };    
    CutscenePhase phase;
};
