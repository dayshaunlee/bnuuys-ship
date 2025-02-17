#include "bnuui/buttons.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"

namespace bnuui {

SquareButton::SquareButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

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

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::PLAY_BUTTON_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

// TODO: Update the Textures appropriately later.
void PlayButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::PLAY_BUTTON_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::PLAY_BUTTON_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

Box::Box(vec2 pos, vec2 scale, float rot) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::SQUARE_3_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void Box::doUpdate(float dt) {
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    if (this->active && this->onActive) {
        this->onActive(*this);
    } 

    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

LongBox::LongBox(vec2 pos, vec2 scale, float rot) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::LONG_BOX;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void LongBox::doUpdate(float dt) {
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    if (this->active && this->onActive) {
        this->onActive(*this);
    } 

    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

PlayerStatus::PlayerStatus(vec2 pos, vec2 scale, float rot, float& val) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
    curr_health = &val;
}

// I realized I could've done this for a lot of the previous anims above, but I'm too lazy.
// Additionally, I'm not sure if placing this here is better or if putting it in Animation System is better.
TEXTURE_ASSET_ID GetNextFaceTexture(TEXTURE_ASSET_ID currAnim) {
    if (currAnim == TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL09)
        return TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01;
    return (TEXTURE_ASSET_ID)((int) currAnim + 1);
}

void PlayerStatus::doUpdate(float dt) {
    // Change to next animation.
    if (time_ms <= 0) {
        this->texture = GetNextFaceTexture(this->texture);
        time_ms = ANIMATION_TIME/1.5f;
    }
    time_ms -= dt;
}

}  // namespace bnuui
