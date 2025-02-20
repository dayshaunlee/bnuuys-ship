#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// the game camera system that controls the view by always following the ship
class CameraSystem {
   public:
    static CameraSystem* GetInstance();
    void setCameraScreen(float accelerationX, float accelerationY);
    void update(float deltaTime);
    void inverse_velocity_x(tson::Vector2f nnorm);
    void inverse_velocity_y(tson::Vector2f nnorm);

   private:
    static CameraSystem* camera;
    vec2 velocity = {0.0f, 0.0f};
    float friction = 0.995f;  // Slow down over time like there's momentum
};