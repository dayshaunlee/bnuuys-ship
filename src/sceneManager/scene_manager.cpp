#include "sceneManager/scene_manager.hpp"
#include "sceneManager/scene.hpp"

void SceneManager::registerScene(Scene* scene) {
    scenes[scene->getName()] = scene;
}

// This creates a REQUEST to switch a scene.
// The reason we don't immediately switch here is to let all systems run first.
// That way we don't have unexpected behaviour.
void SceneManager::switchScene(const std::string& name) {
    // First check if scene exists.
    if (scenes.find(name) != scenes.end()) {
        nextScene = scenes[name];
    }
}

Scene* SceneManager::getCurrentScene() {
    return currScene;
}

// This function will be called every time in main.
// Essentially this checks if we have a switch Scene Request.
// If we do, then we will switch the scene, otherwise do nothing.
void SceneManager::checkSceneSwitch() {
    if (nextScene != nullptr) {
        if (currScene) {
            currScene->Exit();
        }
        currScene = nextScene;
        currScene->Init();
        nextScene = nullptr;
    }
}
