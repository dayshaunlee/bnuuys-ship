#include "modules_system.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <limits>
#include <memory>
#include <ostream>
#include "camera_system.hpp"
#include "common.hpp"
#include "decisionTrees/decision_node.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/entity.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"

bool SearchForEnemy(AutoCannonContext& ctx) {
    if (!registry.motions.has(ctx.cannon_entity)) {
        std::cout << "[SimpleCannon] Seems like the cannon has no motion." << std::endl;
        return false;
    }

    Motion& cannon_motion = registry.motions.get(ctx.cannon_entity);
    vec2 cannon_pos = cannon_motion.position - CameraSystem::GetInstance()->position;

    float smallest_dist = std::numeric_limits<float>::max();
    for (Entity enemy_entity : registry.enemies.entities) {
        if (!registry.motions.has(enemy_entity)) {
            std::cout << "[SimpleCannon] By the way, there are enemies without motion entities" << std::endl;
            continue;
        }
        Motion& enemy_motion = registry.motions.get(enemy_entity);
        vec2 enemy_pos = enemy_motion.position;
        float dist = glm::distance(enemy_pos, cannon_pos);
        if (dist <= 200.0f && dist < smallest_dist) {
            smallest_dist = dist;
            ctx.enemy_entity = enemy_entity;
            ctx.enemy_pos = enemy_pos;
        }
    }

    if (smallest_dist == std::numeric_limits<float>::max()) {
        // No enemy was found.
        return false;
    }

    return true;
}

ModulesSystem::ModulesSystem() {
    // Initialize Decision trees.

    // Simple Cannon Decision Tree.
    auto shootCannon = std::make_shared<DecisionNode<AutoCannonContext>>(
        [](AutoCannonContext& ctx) {
            // Aim the cannon towards the enemy.
            vec2 cannon_pos = registry.motions.get(ctx.cannon_entity).position;
            vec2 enemy_pos = ctx.enemy_pos + CameraSystem::GetInstance()->position;
            registry.motions.get(ctx.cannon_entity).angle = 
                degrees(atan2(enemy_pos.y - cannon_pos.y, enemy_pos.x - cannon_pos.x)) + 90.0f;
            if (registry.simpleCannons.get(ctx.cannon_entity).timer_ms <= 0) {
                createCannonProjectile(cannon_pos, enemy_pos);
                registry.simpleCannons.get(ctx.cannon_entity).timer_ms = SIMPLE_CANNON_COOLDOWN;
            }
            return true;
        }
    );

    auto searchForEnemyNode = std::make_shared<DecisionNode<AutoCannonContext>>(
        [](AutoCannonContext& ctx) {
            return SearchForEnemy(ctx);
        }
    );

    searchForEnemyNode->setBranches(shootCannon, nullptr);
    dtree_AutoCannon = searchForEnemyNode;
}

void ModulesSystem::step(float elapsed_ms) {
    // Simple cannon module.
    for (Entity sc_entity : registry.simpleCannons.entities) {
        SimpleCannon& sc = registry.simpleCannons.get(sc_entity);
        if (sc.timer_ms > 0)
            sc.timer_ms -= elapsed_ms;
        else
            sc.timer_ms = 0;

        // Check automation.
        if (sc.is_automated) {
            AutoCannonContext ctx(sc_entity);
            auto dtree_node = dtree_AutoCannon->decide(ctx);
            while (dtree_node != nullptr) {
                dtree_node->execute(ctx);
                dtree_node = dtree_node->decide(ctx);
            }
        }
    }
}

    
