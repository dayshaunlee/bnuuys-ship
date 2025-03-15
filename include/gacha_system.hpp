#pragma once
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "sceneManager/scene.hpp"
#include "bnuui/buttons.hpp"
#include "scenes/game_level.hpp"
#include <random>
#include <unordered_set>
#include <vector>


class GachaSystem{
    public:

    GachaSystem();
    
    void setLevelPool(int level, const std::vector<MODULE_TYPES>& modulesPool);
    void setDropRate(MODULE_TYPES module, float dropRate);
    std::vector<MODULE_TYPES> getModuleOptions(int level);
    void displayGacha(int level, bnuui::SceneUI& scene_ui, GameLevel& currentLevel);
    // bool isOnDisplay();

    private:
    // bool onDisplay;
    std::vector<std::unordered_set<MODULE_TYPES>> levelModulePools;
    std::unordered_map<MODULE_TYPES, float> moduleDropRates;
    std::default_random_engine rng;
};