#pragma once
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <random>
#include <unordered_set>
#include <vector>


class GachaSystem{
    public:

    GachaSystem();
    
    void setLevelPool(int level, std::vector<MODULE_TYPES>& modulesPool);
    std::vector<MODULE_TYPES> getModuleOptions(int level);
    void displayGacha(int level);


    private:
    std::vector<std::unordered_set<MODULE_TYPES>> levelModulePools;
    std::unordered_map<MODULE_TYPES, float> moduleDropRates;
    std::default_random_engine rng;
};