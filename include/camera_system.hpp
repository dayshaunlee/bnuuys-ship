#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// the game camera system that controls the view by always following the ship
class CameraSystem {
   public:
    static CameraSystem* GetInstance();
    // void setCameraScreen(float accelerationX, float accelerationY);
    void update(float deltaTime);
    void addToCameraVelocity (vec2 cameraVelocity);
    //void inverse_velocity(int ship_x, int ship_y, int island_x, int island_y);
    void inverse_velocity();

    vec2 velocity = {0.0f, 0.0f};
    float friction = 0.995f;  // Slow down over time like there's momentum
    
   private:
    static CameraSystem* camera;

};
