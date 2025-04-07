#pragma once
#include <glm/ext/vector_float2.hpp>
#include "bnuui.hpp"

namespace bnuui {

class SquareButton : public Element {
public:
    SquareButton(vec2 pos, vec2 scale, float rot);
    ~SquareButton() = default;
    void doUpdate(float dt) override;
};

class PlayButton : public Element {
public:
    PlayButton(vec2 pos, vec2 scale, float rot);
    ~PlayButton() = default;
    void doUpdate(float dt) override;
};

class RestartButton : public Element {
    public:
        RestartButton(vec2 pos, vec2 scale, float rot);
        ~RestartButton() = default;
        void doUpdate(float dt) override;
    };

class TutorialButton : public Element {
    public:
        TutorialButton(vec2 pos, vec2 scale, float rot);
        ~TutorialButton() = default;
        void doUpdate(float dt) override;
    };


class ExitButton : public Element {
    public:
        ExitButton(vec2 pos, vec2 scale, float rot);
        ~ExitButton() = default;
        void doUpdate(float dt) override;
    };

class ContinueButton : public Element {
    public:
        ContinueButton(vec2 pos, vec2 scale, float rot);
        ~ContinueButton() = default;
        void doUpdate(float dt) override;
    };

class Book : public Element {
    public:
        Book(vec2 pos, vec2 scale, float rot);
        ~Book() = default;
        void doUpdate(float dt) override;
    };

class Box : public Element {
public:
    Box(vec2 pos, vec2 scale, float rot);
    ~Box() = default;
    void doUpdate(float dt) override;
};

class LongBox : public Element {
public:
    LongBox(vec2 pos, vec2 scale, float rot, bool on_top = false);
    ~LongBox() = default;
    void doUpdate(float dt) override;
};

class PlayerStatus : public Element {
private:
    float* max_health;
    float* curr_health;
    float time_ms;
public:
    PlayerStatus(vec2 pos, vec2 scale, float rot, float& val, float& max_val, bool on_top = false);
    ~PlayerStatus() = default;
    void doUpdate(float dt) override;
};

class SimpleBox : public Element {
public:
    SimpleBox(vec2 pos, vec2 scale, float rot);
    ~SimpleBox() = default;
    void doUpdate(float dt) override;
};

class ProgressBar : public Element {
private:
    float* max_val;
    float* curr_val;
public:
    ProgressBar(vec2 pos, vec2 scale, float rot, float& val, float& max_val);
    ~ProgressBar() = default;
    void doUpdate(float dt) override;
};

class Cursor : public Element {
public:
    Cursor(vec2 pos, vec2 scale, float rot);
    ~Cursor() = default;
    void doUpdate(float dt) override;
};

class TextLabel : public Element {
public:
    TextLabel(vec2 pos, float font_size, const std::string& text, bool on_top = false);
    TextLabel(vec2 pos, float font_size, vec3 color, const std::string& text);
    ~TextLabel() = default;
    void doUpdate(float dt) override;
    void setText(const std::string& text);
};

class DialogueBox : public Element {
    public:
        DialogueBox(vec2 pos, vec2 scale, float rot, bool on_top = false);
        ~DialogueBox() = default;
        void doUpdate(float dt) override;
    };

}
