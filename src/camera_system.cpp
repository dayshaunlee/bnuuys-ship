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

    // TODO for Lily: Add friction here.

    float step_seconds = deltaTime / 1000.f;
    position += vel * step_seconds;
}

void CameraSystem::setToPreviousPosition() {
    position = prev_pos;
}
