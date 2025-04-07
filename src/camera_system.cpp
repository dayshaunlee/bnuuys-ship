#include "camera_system.hpp"
#include <iostream>
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

void CameraSystem::update(float deltaTime) {
    prev_pos = position;
    // std::cout << "x: " << vel.x << " y: "<< vel.y << std::endl;
    float step_seconds = deltaTime / 1000.f;
    // Apply friction when no input is detected
    Camera& c  = registry.cameras.components[0];

    if (c.applyFrictionX) {
        vel.x *= friction;
        if (std::abs(vel.x) < 0.1f) { 
            vel.x = 0.0f;
        }
    }

    if (c.applyFrictionY) {
        vel.y *= friction;
        if (std::abs(vel.y) < 0.1f) { 
            vel.y = 0.0f;
        }
    }
    c.applyFrictionX = true;
    c.applyFrictionY = true;

    position += vel * step_seconds;
}

void CameraSystem::setToPreviousPosition(vec2 normal) {
    position = prev_pos + normal;
}

bool IsEnemyOnScreen() {
    CameraSystem* camera = CameraSystem::GetInstance();
    Camera& c = registry.cameras.components[0];
    for (Entity entity : registry.enemies.entities) {
        Motion& motion = registry.motions.get(entity);
        if (motion.position.x > camera->position.x - WINDOW_WIDTH_PX / 2 &&
            motion.position.x < camera->position.x + WINDOW_WIDTH_PX / 2 &&
            motion.position.y > camera->position.y - WINDOW_HEIGHT_PX / 2 &&
            motion.position.y < camera->position.y + WINDOW_HEIGHT_PX / 2) {
            return true;
        }
    }
    return false;
}