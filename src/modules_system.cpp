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
        if (dist <= SIMPLE_CANNON_AUTO_RANGE && dist < smallest_dist) {
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

bool SearchForEnemy(AutoLaserContext& ctx) {
    if (!registry.motions.has(ctx.laser_entity)) {
        std::cout << "[LaserWeapon] No motion component found." << std::endl;
        return false;
    }

    Motion& laser_motion = registry.motions.get(ctx.laser_entity);
    vec2 laser_pos = laser_motion.position - CameraSystem::GetInstance()->position;

    float smallest_dist = std::numeric_limits<float>::max();
    for (Entity enemy_entity : registry.enemies.entities) {
        if (!registry.motions.has(enemy_entity)) {
            std::cout << "[LaserWeapon] Enemy without motion component detected." << std::endl;
            continue;
        }

        Motion& enemy_motion = registry.motions.get(enemy_entity);
        vec2 enemy_pos = enemy_motion.position;
        float dist = glm::distance(enemy_pos, laser_pos);

        if (dist <= LASER_AUTO_RANGE && dist < smallest_dist) {  // 300 range for lasers
            smallest_dist = dist;
            ctx.enemy_entity = enemy_entity;
            ctx.enemy_pos = enemy_pos;
        }
    }

    return (smallest_dist != std::numeric_limits<float>::max());
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
            SimpleCannon& sc = registry.simpleCannons.get(ctx.cannon_entity);
            if (sc.timer_ms <= 0) {
                if (sc.is_modified) {
                    CannonModifier& cm = registry.cannonModifiers.get(ctx.cannon_entity);
                    createModifiedCannonProjectile(cannon_pos, enemy_pos, cm);
                } else {
                    createCannonProjectile(cannon_pos, enemy_pos);
                }
                sc.timer_ms = SIMPLE_CANNON_COOLDOWN;
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



    auto shootLaser = std::make_shared<DecisionNode<AutoLaserContext>>(
        [](AutoLaserContext& ctx) {
            vec2 laser_pos = registry.motions.get(ctx.laser_entity).position;
            vec2 enemy_pos = ctx.enemy_pos + CameraSystem::GetInstance()->position;
            
            // Set laser direction
            registry.motions.get(ctx.laser_entity).angle =
                degrees(atan2(enemy_pos.y - laser_pos.y, enemy_pos.x - laser_pos.x)) + 90.0f;
            
            LaserWeapon& lw = registry.laserWeapons.get(ctx.laser_entity);
            if (lw.timer_ms <= 0) {
                createLaserBeam(laser_pos, enemy_pos);
                lw.timer_ms = LASER_COOLDOWN;
            }
            return true;
        }
    );

    auto searchForEnemyLaserNode = std::make_shared<DecisionNode<AutoLaserContext>>(
        [](AutoLaserContext& ctx) {
            return SearchForEnemy(ctx);
        }
    );
    
    searchForEnemyLaserNode->setBranches(shootLaser, nullptr);
    dtree_AutoLaser = searchForEnemyLaserNode;
    
}

void ModulesSystem::step(float elapsed_ms) {
    // Simple cannon module.
    for (Entity sc_entity : registry.simpleCannons.entities) {
        SimpleCannon& sc = registry.simpleCannons.get(sc_entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(sc_entity).used_texture;
        if (sc.timer_ms > 0) {
            sc.timer_ms -= elapsed_ms;
            if (!sc.is_modified) {
                texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01_SHADED;
            } else {
                texture = TEXTURE_ASSET_ID::BUBBLE_CANNON_SHADED;       // if have more cannon mods, this needs to be edited
            }
        } else {
            sc.timer_ms = 0;
            if (!sc.is_modified) {
                texture = TEXTURE_ASSET_ID::SIMPLE_CANNON01;
            } else {
                texture = TEXTURE_ASSET_ID::BUBBLE_CANNON;              // if have more cannon mods, this needs to be edited
            }
        }

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

    for (Entity lw_entity : registry.laserWeapons.entities) {
        LaserWeapon& lw = registry.laserWeapons.get(lw_entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(lw_entity).used_texture;
        if (lw.timer_ms > 0) {
            lw.timer_ms -= elapsed_ms;
            texture = TEXTURE_ASSET_ID::LASER_WEAPON0_SHADED;
        } else {
            lw.timer_ms = 0;
            texture = TEXTURE_ASSET_ID::LASER_WEAPON0;
        }
    
        // Check automation.
        if (lw.is_automated) {
            AutoLaserContext ctx(lw_entity);
            auto dtree_node = dtree_AutoLaser->decide(ctx);
            while (dtree_node != nullptr) {
                dtree_node->execute(ctx);
                dtree_node = dtree_node->decide(ctx);
            }
        }
    }
    

    // update the laser beams to follow the ship (unlike projectiles)

    for (Entity beam : registry.laserBeams.entities) {
        Motion& lb_motion = registry.motions.get(beam);
        LaserBeam& laser_beam = registry.laserBeams.get(beam);
        vec2 currentCamloc = CameraSystem::GetInstance()->position; 
        vec2 shipDistanceMoved = currentCamloc - laser_beam.prevCamPos;
        lb_motion.position -= shipDistanceMoved;
        laser_beam.prevCamPos = currentCamloc;
    }

    // Heal module
    for (Entity heal_entity : registry.healModules.entities) {
        Heal& heal = registry.healModules.get(heal_entity);
        TEXTURE_ASSET_ID& texture = registry.renderRequests.get(heal_entity).used_texture;
        if (heal.cooldown_ms > 0) {
            heal.cooldown_ms -= elapsed_ms;
            texture = TEXTURE_ASSET_ID::HEAL_MODULE_SHADED;
        } else {
            heal.cooldown_ms = 0;
            texture = TEXTURE_ASSET_ID::HEAL_MODULE;
        }
            

        if (heal.is_automated && heal.cooldown_ms <= 0) {
            assert(registry.ships.size() == 1);
            Ship& ship = registry.ships.components[0];
            ship.health = std::min(ship.health + HEAL_AMOUNT, ship.maxHealth);
            heal.cooldown_ms = HEAL_COOLDOWN;
        }
    }
}

    
