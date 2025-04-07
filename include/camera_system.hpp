#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// BASIC CREATIVE FEATURE: CAMERA CONTROLS

// the game camera system that controls the view by always following the ship
class CameraSystem {
   public:
    static CameraSystem* GetInstance();
    void update(float deltaTime);
    void setToPreviousPosition(vec2 normal);
    bool IsEnemyOnScreen();

    vec2 position = {0.0f, 0.0f};
    vec2 prev_pos = {0.0f, 0.0f};
    vec2 vel = {0.0f, 0.0f};

   private:
    static CameraSystem* camera;
    float friction = 0.99f;  // Slow down over time like there's momentum

};