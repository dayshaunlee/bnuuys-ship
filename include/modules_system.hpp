#pragma once
// A simple module system that runs all the modules.
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include "decisionTrees/decision_node.hpp"
#include "tinyECS/entity.hpp"

struct AutoCannonContext {
    Entity cannon_entity;
    Entity enemy_entity;
    glm::vec2 enemy_pos;

    AutoCannonContext(Entity c_entity) : cannon_entity(c_entity) {}
};

struct AutoLaserContext {
    Entity laser_entity;
    Entity enemy_entity;
    glm::vec2 enemy_pos;

    AutoLaserContext(Entity l_entity) : laser_entity(l_entity) {}
};

struct AutoHealContext {
    Entity heal_entity;
    Entity ship_entity;
    AutoHealContext(Entity h_entity) : heal_entity(h_entity) {}
};

class ModulesSystem {
   private:
    std::shared_ptr<DecisionNode<AutoCannonContext>> dtree_AutoCannon;
    std::shared_ptr<DecisionNode<AutoLaserContext>> dtree_AutoLaser;
    
   public:
    void step(float elapsed_ms);

    ModulesSystem();
};
