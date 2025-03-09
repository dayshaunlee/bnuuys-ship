#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "sceneManager/scene.hpp"

/*
 *   Probably bad practice to make this a singleton, but it does make it easier
 *   to access considering the previous layout of A1.
 */

class SceneManager {

private:
    std::unordered_map<std::string, Scene*> scenes;
    Scene* currScene = nullptr;
    Scene* nextScene = nullptr;
    // For restarting back at level scene after death
    std::string prevSceneName;
    std::string nextLevelName;

    SceneManager() = default;
    ~SceneManager() = default;

public:
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void registerScene(Scene* s);

    void checkSceneSwitch();

    void switchScene(const std::string& name);

    Scene* getCurrentScene();  
    std::string getNewLevelSceneName();

    void setRestartScence(const std::string& restartName);
    void setNextLevelScence(const std::string& nextLevelName);

    void restartScene();
};
