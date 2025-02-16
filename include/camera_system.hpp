#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// // the game camera system that controls the view by always following the ship
// class CameraSystem {
//    public:
//     vec2 cameraPosition;

//     static CameraSystem* GetInstance();
//     void DestroyInstance();
//     float SetCameraScale(float scale);
//     // vec2 GetCameraPosition();
//     void setCameraScreen(float x, float y);
//     vec2 GetCameraScreen();

//    private:
//     static CameraSystem* camera;
//     float cameraScale;
//     vec2 cameraScreen;
// };

class CameraSystem {
   public:
    static CameraSystem* GetInstance();
    void setCameraScreen(float accelerationX, float accelerationY);
    void update(float deltaTime);

   private:
    static CameraSystem* camera;
    vec2 velocity = {0.0f, 0.0f};
    float friction = 0.995f;  // Slow down over time like there's momentum
};