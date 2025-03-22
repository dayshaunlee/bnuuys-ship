#pragma once
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "sceneManager/scene.hpp"
#include "bnuui/buttons.hpp"
#include "scenes/game_level.hpp"
#include "world_init.hpp"
#include <random>
#include <unordered_set>
#include <vector>

class GachaSystem {
public:
    static GachaSystem& getInstance(); // Singleton instance access

    void setLevelPool(int level, const std::vector<MODULE_TYPES>& modulesPool);
    void setDropRate(MODULE_TYPES module, float dropRate);
    std::vector<MODULE_TYPES> getModuleOptions(int level);
    void handleOptionClick(MODULE_TYPES moduleChose);
    void displayGacha(int level, bnuui::SceneUI& scene_ui, GameLevel& currentLevel);

    GachaSystem(const GachaSystem&) = delete;
    GachaSystem& operator=(const GachaSystem&) = delete;

private:
    GachaSystem(); 

    std::vector<std::unordered_set<MODULE_TYPES>> levelModulePools;
    std::unordered_map<MODULE_TYPES, float> moduleDropRates;
    std::default_random_engine rng;

    // Scuffed implementation but ig it's fine.
    std::vector<bool> hovered_options = {false, false, false};

    std::string tooltip_str = " ";
    std::shared_ptr<bnuui::Box> createModuleOption(MODULE_TYPES moduleType, vec2 position, 
                                                   bnuui::SceneUI& scene_ui, GameLevel& currentLevel);
};
