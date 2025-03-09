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

class ModulesSystem {
   private:
    std::shared_ptr<DecisionNode<AutoCannonContext>> dtree_AutoCannon;

   public:
    void step(float elapsed_ms);

    ModulesSystem();
};
