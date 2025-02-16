#include "sceneManager/scene_manager.hpp"
#include "sceneManager/scene.hpp"

void SceneManager::registerScene(Scene* scene) {
    scenes[scene->getName()] = scene;
}

void SceneManager::switchScene(const std::string& name) {
    // First check if scene exists.
    if (scenes.find(name) != scenes.end()) {
        if (currScene) currScene->Exit();   // Trigger whatever.
    }
    currScene = scenes[name];
    currScene->Init();
}

Scene* SceneManager::getCurrentScene() {
    return currScene;
}

