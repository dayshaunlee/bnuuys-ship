#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// the game camera system that controls the view by always following the ship
class CameraSystem {
   public:
    static CameraSystem* GetInstance();
    void update(float deltaTime);
    //void inverse_velocity(int ship_x, int ship_y, int island_x, int island_y);
    void setToPreviousPosition();

    vec2 position = {0.0f, 0.0f};
    vec2 prev_pos = {0.0f, 0.0f};
    vec2 vel = {0.0f, 0.0f};

   private:
    static CameraSystem* camera;
    float friction = 0.995f;  // Slow down over time like there's momentum
};
