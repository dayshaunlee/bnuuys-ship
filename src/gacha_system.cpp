#include "modules_system.hpp"
#include "camera_system.hpp"
#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "gacha_system.hpp"
#include "sceneManager/scene_manager.hpp"

GachaSystem::GachaSystem(){
    rng.seed(static_cast<unsigned>(time(nullptr)));
    levelModulePools.resize(5); // temporarily set to 5 levels for now

    //TODO lily set level pool and set module drop rates
    setLevelPool(0, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::FAST_CANNON});
    setLevelPool(1, {MODULE_TYPES::SIMPLE_CANNON, MODULE_TYPES::FAST_CANNON, MODULE_TYPES::PLATFORM});
    
    // the rates will be normalized if not set the default is 1
    setDropRate(MODULE_TYPES::STEERING_WHEEL, 0);
    setDropRate(MODULE_TYPES::HELPER_BUNNY, 0);
    setDropRate(MODULE_TYPES::EMPTY, 0);
    setDropRate(MODULE_TYPES::PLATFORM, 5);
    setDropRate(MODULE_TYPES::SIMPLE_CANNON, 100);
    setDropRate(MODULE_TYPES::FAST_CANNON, 50);
}


void GachaSystem::setLevelPool(int level, const std::vector<MODULE_TYPES>& modulesPool){
    if (level >= levelModulePools.size()) {
        levelModulePools.resize(level + 1);
    }
    levelModulePools[level] = std::unordered_set<MODULE_TYPES>(modulesPool.begin(), modulesPool.end());
}

void GachaSystem::setDropRate(MODULE_TYPES module, float dropRate) {
    moduleDropRates[module] = dropRate;
}

std::vector<MODULE_TYPES> GachaSystem::getModuleOptions(int level){
    int numOptions = 3;

    if (level >= levelModulePools.size() || levelModulePools[level].empty()) {
        return {};
    }

    std::vector<MODULE_TYPES> availableModules(levelModulePools[level].begin(), levelModulePools[level].end());
    std::vector<float> weights;

    // Assign weights based on drop rate (default to 1.0 if not set)
    for (MODULE_TYPES module : availableModules) {
        weights.push_back(moduleDropRates.count(module) ? moduleDropRates[module] : 1.0f);
    }

    // Normalize weights to create a probability distribution
    float sumWeights = std::accumulate(weights.begin(), weights.end(), 0.0f);
    for (float& weight : weights) {
        weight /= sumWeights;
    }

    // Weighted random selection
    std::vector<MODULE_TYPES> selectedModules;
    std::discrete_distribution<int> distribution(weights.begin(), weights.end());

    for (int i = 0; i < numOptions; ++i) {
        int index = distribution(rng);
        selectedModules.push_back(availableModules[index]);
    }

    return selectedModules;
}

void GachaSystem::displayGacha(int level){
    std::vector<MODULE_TYPES> threeOptions = getModuleOptions(level);
    
}


