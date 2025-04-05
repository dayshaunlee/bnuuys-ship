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
    /*std::cout << "position: " << position.x << ", " << position.y << std::endl;
    std::cout << "prev_pos: " << prev_pos.x << ", " << prev_pos.y << std::endl;*/
    std::cout << "HI" << std::endl;
    //if (length(position - prev_pos) < 0.0001f || glm::dot(normal, normal) < 0.0001f) {
    //    return;  // avoid division by 0
    //}
    vec2 ship_vector = position - prev_pos;
    ship_vector = normalize(ship_vector);
    // component of ship direction that is perpendicular to wall, parallel to the normal
    vec2 perpendicular_vec = (glm::dot(ship_vector, normal) / glm::dot(normal, normal)) * normal;

    vec2 parallel_vec = (ship_vector - perpendicular_vec) * length(position - prev_pos);
    std::cout << "ship_vector: " << ship_vector.x << ", " << ship_vector.y << std::endl;
    std::cout << "normal (mtv): " << normal.x << ", " << normal.y << std::endl;
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