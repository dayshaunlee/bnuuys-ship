#include "bnuui/buttons.hpp"
#include "tinyECS/components.hpp"

namespace bnuui {

SquareButton::SquareButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0,0}; 
    this->color = {1,1,1};

    this->texture = TEXTURE_ASSET_ID::SQUARE_3_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void SquareButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering) {
        this->texture = TEXTURE_ASSET_ID::SQUARE_3_HOVER;
        if (this->onHover) {
            this->onHover(*this);
        }
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::SQUARE_3_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::SQUARE_3_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

PlayButton::PlayButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0,0}; 
    this->color = {1,1,1};

    // TODO: Update this into Play button texture when you have time to import.
    this->texture = TEXTURE_ASSET_ID::SQUARE_3_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;

}

// TODO: Update the Textures appropriately later.
void PlayButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering) {
        this->texture = TEXTURE_ASSET_ID::SQUARE_3_HOVER;
        if (this->onHover) {
            this->onHover(*this);
        }
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::SQUARE_3_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

}
