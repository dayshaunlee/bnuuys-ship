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

class Box : public Element {
public:
    Box(vec2 pos, vec2 scale, float rot);
    ~Box() = default;
    void doUpdate(float dt) override;
};

class LongBox : public Element {
public:
    LongBox(vec2 pos, vec2 scale, float rot);
    ~LongBox() = default;
    void doUpdate(float dt) override;
};

class PlayerStatus : public Element {
private:
    float max_health;
    float* curr_health;
    float time_ms;
public:
    PlayerStatus(vec2 pos, vec2 scale, float rot, float& val, float max_val);
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
    float max_val;
    float* curr_val;
public:
    ProgressBar(vec2 pos, vec2 scale, float rot, float& val, float max_val);
    ~ProgressBar() = default;
    void doUpdate(float dt) override;
};

class Cursor : public Element {
public:
    Cursor(vec2 pos, vec2 scale, float rot);
    ~Cursor() = default;
    void doUpdate(float dt) override;
};

}
