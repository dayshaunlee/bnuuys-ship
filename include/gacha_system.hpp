#pragma once
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

class GachaSystem(){
    public:
    
    setLevelPool();
    getModuleOptions();
    displayGacha();
    private:
    std::vector<std::unordered_set> levelModulePool;

};