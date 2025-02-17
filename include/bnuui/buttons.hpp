#pragma once
#include <glm/ext/vector_float2.hpp>
#include "bnuui.hpp"

namespace bnuui {

class SquareButton : public Element {
public:
    SquareButton(vec2 pos, vec2 scale, float rot);
    void doUpdate(float dt) override;
};

class PlayButton : public Element {
public:
    PlayButton(vec2 pos, vec2 scale, float rot);
    void doUpdate(float dt) override;
};

}
