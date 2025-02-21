#include "camera_system.hpp"
#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"

CameraSystem* CameraSystem::camera = 0;

CameraSystem* CameraSystem::GetInstance() {
    if (camera == 0) {
        camera = new CameraSystem();
    }
    return camera;
}

void CameraSystem::setCameraScreen(float accelerationX, float accelerationY) {
    velocity.x += accelerationX;
    velocity.y += accelerationY;

    // Limit velocity to max speed
    velocity.x = std::clamp(velocity.x, -SHIP_CAMERA_SPEED, SHIP_CAMERA_SPEED);
    velocity.y = std::clamp(velocity.y, -SHIP_CAMERA_SPEED, SHIP_CAMERA_SPEED);
}

void CameraSystem::update(float deltaTime) {
    velocity.x *= friction;
    velocity.y *= friction;

    // Stop when velocity is near zero
    if (std::abs(velocity.x) < 0.01f) velocity.x = 0.0f;
    if (std::abs(velocity.y) < 0.01f) velocity.y = 0.0f;

    // Apply movement to background objects
    for (Entity backObject : registry.backgroundObjects.entities) {
        Motion& objectMotion = registry.motions.get(backObject);
        objectMotion.velocity.x = velocity.x;
        objectMotion.velocity.y = velocity.y;
    }
}

//void CameraSystem::inverse_velocity(int ship_x, int ship_y, int island_x, int island_y) {
//    if (ship_x <= island_x) {
//        if (velocity.x <= 0) {
//            velocity.y = -velocity.y;
//        } else {
//            velocity.x = -velocity.x;
//            velocity.y = -velocity.y;
//        }
//    } else {
//        if (velocity.x >= 0) {
//            velocity.y = -velocity.y;
//        } else {
//            velocity.x = -velocity.x;
//            velocity.y = -velocity.y;
//        }
//    }
//}

void CameraSystem::inverse_velocity() {
    velocity.x = -velocity.x;
    velocity.y = -velocity.y;
}