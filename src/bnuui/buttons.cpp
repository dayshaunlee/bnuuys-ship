#include "bnuui/buttons.hpp"
#include <glm/common.hpp>
#include <memory>
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

ExitButton::ExitButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::EXIT_BUTTON_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

// TODO: Update the Textures appropriately later.
void ExitButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::EXIT_BUTTON_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::EXIT_BUTTON_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

RestartButton::RestartButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::RESTART_BUTTON_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void RestartButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::RESTART_BUTTON_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::RESTART_BUTTON_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

TutorialButton::TutorialButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::TUTORIAL_BUTTON_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void TutorialButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::TUTORIAL_BUTTON_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::TUTORIAL_BUTTON_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}

ContinueButton::ContinueButton(vec2 pos, vec2 scale, float rot) {
    // Decorate the square button.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::CONTINUE_BUTTON_NORMAL;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void ContinueButton::doUpdate(float dt) {
    // Check if it's being hovered/hot.
    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    // Check if it's being clicked.
    if (this->active) {
        this->texture = TEXTURE_ASSET_ID::CONTINUE_BUTTON_CLICKED;
        if (this->onActive) {
            this->onActive(*this);
        }
    }

    if (!this->hovering && !this->active) {
        this->texture = TEXTURE_ASSET_ID::CONTINUE_BUTTON_NORMAL;
    }

    // Update if needed.
    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }
}


Book::Book(vec2 pos, vec2 scale, float rot) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::BOOK;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void Book::doUpdate(float dt) {
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

LongBox::LongBox(vec2 pos, vec2 scale, float rot, bool on_top) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::LONG_BOX;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
    this->over_overlay = on_top;
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

PlayerStatus::PlayerStatus(vec2 pos, vec2 scale, float rot, float& val, float& max_val, bool on_top): max_health(&max_val), curr_health(&val) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
    this->over_overlay = on_top;
}

inline bool isNeutralUIFace(TEXTURE_ASSET_ID anim) {
    return anim >= TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01 && anim <= TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL09;
}

inline bool isAngryUIFace(TEXTURE_ASSET_ID anim) {
    return anim >= TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY01 && anim <= TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY05;
}

// I realized I could've done this for a lot of the previous anims above, but I'm too lazy.
// Additionally, I'm not sure if placing this here is better or if putting it in Animation System is better.
TEXTURE_ASSET_ID GetNextFaceTexture(TEXTURE_ASSET_ID currAnim, float percentage) {
    if (percentage > 0.5f) {
        if (isAngryUIFace(currAnim)) return TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01;
        if (currAnim == TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL09) return TEXTURE_ASSET_ID::BUNNY_FACE_NEUTRAL01;
        return (TEXTURE_ASSET_ID) ((int) currAnim + 1);
    } else {
        if (isNeutralUIFace(currAnim)) return TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY01;
        // Angry face.
        if (currAnim == TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY05) return TEXTURE_ASSET_ID::BUNNY_FACE_ANGRY01;
        return (TEXTURE_ASSET_ID) ((int) currAnim + 1);
    }
}

void PlayerStatus::doUpdate(float dt) {
    // Change to next animation.
    float percentage = *curr_health / *max_health;

    if (time_ms <= 0) {
        this->texture = GetNextFaceTexture(this->texture, percentage);
        time_ms = ANIMATION_TIME / 1.5f;
    }

    if (this->hovering && this->onHover) {
        this->onHover(*this);
    }

    if (this->active && this->onActive) {
        this->onActive(*this);
    }

    if (this->onUpdate) {
        this->onUpdate(*this, dt);
    }

    time_ms -= dt;
}

SimpleBox::SimpleBox(vec2 pos, vec2 scale, float rot) {
    // Creating the background for progress bar.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {0, 0, 0};
    this->texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    this->effect = EFFECT_ASSET_ID::EGG;
    this->geometry = GEOMETRY_BUFFER_ID::UI_SQUARE;
    this->visible = true;
}

void SimpleBox::doUpdate(float dt) {
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

ProgressBar::ProgressBar(vec2 pos, vec2 scale, float rot, float& val, float& max_val)
    : max_val(&max_val), curr_val(&val) {
    // Creating the background for progress bar.
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {0.25f, 0.25f, 0.25f};  // Background color.
    this->texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    this->effect = EFFECT_ASSET_ID::EGG;
    this->geometry = GEOMETRY_BUFFER_ID::UI_SQUARE;
    this->visible = true;

    // Now create a child component for the ACTUAL progress bar.
    auto progress_bar = std::make_shared<SimpleBox>(pos, scale, rot);
    progress_bar->color = {0, 1.0f, 0};  // Change later.

    this->children.push_back(progress_bar);
};

glm::vec3 interpolateColor(float t) {
    // Define the colors
    glm::vec3 red(1.0f, 0.0f, 0.0f);    // Red
    glm::vec3 green(0.0f, 1.0f, 0.0f);  // Green

    // Perform linear interpolation
    glm::vec3 result = red + t * (green - red);

    return result;
}

void ProgressBar::doUpdate(float dt) {
    auto progress_bar = this->children[0];
    // First calculate the actual progress.
    float percentage = *curr_val / *max_val;

    // Rescale the progress_bar to percentage * background.
    float new_x = percentage * this->scale.x;
    float diff = this->scale.x - new_x;
    progress_bar->scale.x = new_x;

    progress_bar->color = interpolateColor(percentage);

    // Then move the position by that difference.
    progress_bar->position.x = (this->position.x - diff / 2);

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

Cursor::Cursor(vec2 pos, vec2 scale, float rot) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1.0f, 1.0f, 1.0f};
    this->texture = TEXTURE_ASSET_ID::TILE_CURSOR;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
}

void Cursor::doUpdate(float dt) {
}

TextLabel::TextLabel(vec2 pos, float font_size, const std::string& text, bool on_top) {
    this->position = pos;
    this->rotation = 0.0f;
    this->text = text;
    this->font_size = font_size;

    this->scale = {0,0};    // When I have time to implement the Map

    this->offset = {0, 0};
    this->color = {0,0,0};

    this->visible = true;
    this->over_overlay = on_top;
}

TextLabel::TextLabel(vec2 pos, float font_size, vec3 color, const std::string& text) {
    this->position = pos;
    this->rotation = 0.0f;
    this->text = text;
    this->font_size = font_size;
    this->color = color;

    this->scale = {0,0};    // When I have time to implement the Map
    std::cout << this->scale.x << ' ' << this->scale.y << '\n';

    this->offset = {0, 0};

    this->visible = true;
}

void TextLabel::doUpdate(float dt) {
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

void TextLabel::setText(const std::string& text) {
    this->text = text;
}

DialogueBox::DialogueBox(vec2 pos, vec2 scale, float rot, bool on_top) {
    this->position = pos;
    this->scale = scale;
    this->rotation = rot;

    this->offset = {0, 0};
    this->color = {1, 1, 1};

    this->texture = TEXTURE_ASSET_ID::TUTORIAL_DIALOGUE_BOX_UI;
    this->effect = EFFECT_ASSET_ID::TEXTURED;
    this->geometry = GEOMETRY_BUFFER_ID::SPRITE;
    this->visible = true;
    this->over_overlay = on_top;
}

void DialogueBox::doUpdate(float dt) {
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


}  // namespace bnuui
