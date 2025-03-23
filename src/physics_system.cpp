// internal
#include "physics_system.hpp"

#include <iostream>

#include "camera_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"

// #include "camera_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the
// entity
vec2 get_bounding_box(const Motion& motion) {
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

// ADVANCED CREATIVE FEATURE: PRECISE COLLISION

// LINE/LINE
bool lineLine(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
    float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    if (denom == 0) return false;

    // calculate the direction of the lines
    float uA = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
    float uB = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

    // if uA and uB are between 0-1, lines are colliding
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {
        return true;
    }
    return false;
}

// POLYGON/LINE
bool polyLine(std::vector<tson::Vector2i> vertices, int x1, int y1, int x2, int y2) {
    // go through each of the vertices, plus the next
    // vertex in the list
    int next = 0;
    for (int current = 0; current < vertices.size(); current++) {
        // get next vertex in list
        // if we've hit the end, wrap around to 0
        next = current + 1;
        if (next == vertices.size()) next = 0;

        // get the std::vector<tson::Vector2i> at our current position
        // extract X/Y coordinates from each
        int x3 = vertices[current].x;
        int y3 = vertices[current].y;
        int x4 = vertices[next].x;
        int y4 = vertices[next].y;

        // do a Line/Line comparison
        // if true, return 'true' immediately and
        // stop testing (faster)
        bool hit = lineLine(x1, y1, x2, y2, x3, y3, x4, y4);
        if (hit) {
            return true;
        }
    }

    // never got a hit
    return false;
}

// POLYGON/POINT
// used only to check if the second polygon is
// INSIDE the first
bool polyPoint(std::vector<tson::Vector2i> vertices, float px, float py) {
    bool collision = false;

    // go through each of the vertices, plus the next
    // vertex in the list
    int next = 0;
    for (int current = 0; current < vertices.size(); current++) {
        // get next vertex in list
        // if we've hit the end, wrap around to 0
        next = current + 1;
        if (next == vertices.size()) next = 0;

        // get the tson::Vector2i at our current position
        // this makes our if statement a little cleaner
        tson::Vector2i vc = vertices[current];  // c for "current"
        tson::Vector2i vn = vertices[next];     // n for "next"

        // compare position, flip 'collision' variable
        // back and forth
        if (((vc.y > py && vn.y < py) || (vc.y < py && vn.y > py)) &&
            (px < (vn.x - vc.x) * (py - vc.y) / (vn.y - vc.y) + vc.x)) {
            collision = !collision;
        }
    }
    return collision;
}

// POLYGON/POLYGON: all of this along with helpers from
// (https://www.jeffreythompson.org/collision-detection/poly-poly.php)
bool polyPoly(std::vector<tson::Vector2i> p1, std::vector<tson::Vector2i> p2) {
    // go through each of the vertices, plus the next
    // vertex in the list
    int next = 0;
    for (int current = 0; current < p1.size(); current++) {
        // get next vertex in list
        // if we've hit the end, wrap around to 0
        next = current + 1;
        if (next == p1.size()) next = 0;

        // get the tson::Vector2i at our current position
        // this makes our if statement a little cleaner
        tson::Vector2i vc = p1[current];  // c for "current"
        tson::Vector2i vn = p1[next];     // n for "next"

        // now we can use these two points (a line) to compare
        // to the other polygon's vertices using polyLine()
        bool collision = polyLine(p2, vc.x, vc.y, vn.x, vn.y);
        if (collision) return true;
    }

    return false;
}

bool polyPolyInside(std::vector<tson::Vector2i> p1, std::vector<tson::Vector2i> p2) { // only true if one polygon is completely in the other
    // go through each of the vertices, plus the next
    // vertex in the list
    // check if the 2nd polygon is INSIDE the first
    bool collision = true;

    // works by checking if ALL the points of p2 are INSIDE p1
    for (tson::Vector2i p : p2) {
        if (!polyPoint(p1, p.x, p.y)) collision = false;
    }
    return collision;
}

// Collision using "axis-aligned" rectangular bounding boxes
bool collidesAABB(const Motion& motion1, const Motion& motion2) {
    vec2 half_size1 = get_bounding_box(motion1) / 2.f;
    vec2 half_size2 = get_bounding_box(motion2) / 2.f;

    vec2 pos1 = motion1.position;
    vec2 pos2 = motion2.position - CameraSystem::GetInstance()->position;

    if (pos1.x + half_size1.x < pos2.x - half_size2.x || pos2.x + half_size2.x < pos1.x - half_size1.x) {
        return false;
    }

    if (pos1.y + half_size1.y < pos2.y - half_size2.y || pos2.y + half_size2.y < pos1.y - half_size1.y) {
        return false;
    }

    return true;
}


// This is a SUPER APPROXIMATE check that puts a circle around the bounding
// boxes and sees if the center point of either object is inside the other's
// bounding-box-circle. You can surely implement a more accurate detection
// Clare's note: add camera offset in calculation
// Dayshaun's note: removed camera offset because made projectiles background offset
bool collidesSpherical(const Motion& motion1, const Motion& motion2) {
    vec2 dp = motion1.position - motion2.position;
    float dist_squared = dot(dp, dp);
    const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
    const float other_r_squared = dot(other_bonding_box, other_bonding_box);
    const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
    const float my_r_squared = dot(my_bonding_box, my_bonding_box);
    const float r_squared = max(other_r_squared, my_r_squared);
    if (dist_squared < r_squared) return true;
    return false;
}

bool collidesSphericalShip(const Entity e1, const Entity e2) {
    Motion shipMotion;
    Motion otherMotion;
    if (registry.ships.has(e1)) {
        otherMotion = registry.motions.get(e2);
        shipMotion = registry.motions.get(e1);
    } else {
        otherMotion= registry.motions.get(e1);
        shipMotion= registry.motions.get(e2);
    }

    otherMotion.position += CameraSystem::GetInstance()->position;


    vec2 dp = otherMotion.position - shipMotion.position;
    float dist_squared = dot(dp, dp);
    const vec2 other_bonding_box = get_bounding_box(otherMotion) / 2.f;
    const float other_r_squared = dot(other_bonding_box, other_bonding_box);
    const vec2 my_bonding_box = get_bounding_box(shipMotion) / 2.f;
    const float my_r_squared = dot(my_bonding_box, my_bonding_box);
    const float r_squared = max(other_r_squared, my_r_squared);
    if (dist_squared < r_squared) return true;
    return false;
}

// Brian's Additional Feedback: I added the Camera Offset, but it might not be the EXACT outputs.
// Polygon - Polygon collision
// TODO Dayshaun: this function is mega scuffed and has repeating code WILL FIX SOON TM
bool collidesPoly(const Entity e1, const Entity e2) {
    // discard if the bounding boxes do not collide
    Motion& e1_mot = registry.motions.get(e1);
    Motion& e2_mot = registry.motions.get(e2);
    std::vector<tson::Vector2i> islandPolygon;
    std::vector<tson::Vector2i> basePolygon;
    std::vector<tson::Vector2i> shipPolygon;
    if (registry.islands.has(e1)) {  // e1 is the Island, e2 is the Ship
        islandPolygon = registry.islands.get(e1).polygon;
        shipPolygon = get_poly_from_motion(e2_mot);
        for (auto& p : islandPolygon) {  // account for camera affecting position
            p.x += e1_mot.position.x + CameraSystem::GetInstance()->position.x;
            p.y += e1_mot.position.y + CameraSystem::GetInstance()->position.y;
        }
        if (!collidesAABB(registry.motions.get(e1), registry.motions.get(e2))) return false;
        return polyPoly(islandPolygon, shipPolygon);
    } else if (registry.islands.has(e2)) {  // e2 is the Island, e1 is the Ship
        islandPolygon = registry.islands.get(e2).polygon;
        shipPolygon = get_poly_from_motion(e1_mot);
        for (auto& p : islandPolygon) {  // account for camera affecting position
            p.x += e2_mot.position.x + CameraSystem::GetInstance()->position.x;
            p.y += e2_mot.position.y + CameraSystem::GetInstance()->position.y;
        }
        if (!collidesAABB(registry.motions.get(e2), registry.motions.get(e1))) return false;
        return polyPoly(islandPolygon, shipPolygon);
    } else if (registry.base.has(e1)) {  // e1 is the Base, e2 is the Ship
        basePolygon = registry.base.get(e1).polygon;
        shipPolygon = get_poly_from_motion(e2_mot);
        for (auto& p : basePolygon) {  // account for camera affecting position
            p.x += e1_mot.position.x + CameraSystem::GetInstance()->position.x;
            p.y += e1_mot.position.y + CameraSystem::GetInstance()->position.y;
        }
        if (!collidesAABB(registry.motions.get(e1), registry.motions.get(e2))) return false;
        return polyPolyInside(basePolygon, shipPolygon);
    } else {  // e2 is the Base, e1 is the Ship
        basePolygon = registry.base.get(e2).polygon;
        shipPolygon = get_poly_from_motion(e1_mot);
        for (auto& p : basePolygon) {  // account for camera affecting position
            p.x += e2_mot.position.x + CameraSystem::GetInstance()->position.x;
            p.y += e2_mot.position.y + CameraSystem::GetInstance()->position.y;
        }
        if (!collidesAABB(registry.motions.get(e2), registry.motions.get(e1))) return false;
        return polyPolyInside(basePolygon, shipPolygon);
    }
    // should never reach
    return false;
}

std::vector<tson::Vector2i> get_poly_from_node_pos(ivec2 node_pos) {
    std::vector<tson::Vector2i> polygon;
    int posX = node_pos.x * GRID_CELL_WIDTH_PX + GRID_CELL_WIDTH_PX / 2;
    int posY = node_pos.y * GRID_CELL_HEIGHT_PX + GRID_CELL_HEIGHT_PX / 2;
    int halfWidth = GRID_CELL_WIDTH_PX / 2;
    int halfHeight = GRID_CELL_HEIGHT_PX / 2;

    // top left
    polygon.push_back(tson::Vector2i(posX - halfWidth, posY - halfHeight));
    // top right
    polygon.push_back(tson::Vector2i(posX + halfWidth, posY - halfHeight));
    // bottom right
    polygon.push_back(tson::Vector2i(posX + halfWidth, posY + halfHeight));
    // bottom left
    polygon.push_back(tson::Vector2i(posX - halfWidth, posY + halfHeight));

    return polygon;
}

bool PhysicsSystem::collidesPolyVec(Entity island_entity, ivec2 node_pos) {
    Motion& island_motion = registry.motions.get(island_entity);
    std::vector<tson::Vector2i> islandPolygon;
    std::vector<tson::Vector2i> nodePolygon;
    islandPolygon = registry.islands.get(island_entity).polygon;
    nodePolygon = get_poly_from_node_pos(node_pos);
    for (auto& p : islandPolygon) {
        p.x += island_motion.position.x;
        p.y += island_motion.position.y;
    }

    return polyPoly(islandPolygon, nodePolygon);
}

void PhysicsSystem::step(float elapsed_ms) {
    // TODO: Updates camera and move all the background objects

    // Move each entity that has motion.
    auto& motion_registry = registry.motions;
    for (uint i = 0; i < motion_registry.size(); i++) {
        Motion& motion = motion_registry.components[i];
        Entity entity = motion_registry.entities[i];
        float step_seconds = elapsed_ms / 1000.f;
        motion.position += motion.velocity * step_seconds;

        // Player - Ship collision: Don't allow Player to walk outside of the ship boundaries
        if (registry.ships.components.size() > 0 && registry.players.has(entity)) {
            Motion ship_mot = registry.motions.get(registry.ships.entities[0]);
            motion.position.x = std::clamp(motion.position.x,
                                           ship_mot.position.x - (ship_mot.scale.x / 2) + 16,
                                           ship_mot.position.x + (ship_mot.scale.x / 2) - 16);
            motion.position.y = std::clamp(motion.position.y,
                                           ship_mot.position.y - (ship_mot.scale.y / 2) + 16,
                                           ship_mot.position.y + (ship_mot.scale.y / 2) - 16);
        } else if (registry.walkingPaths.has(entity) && registry.enemies.has(entity)) {
            // walk the path, tile by tile until it reach the end
			WalkingPath& walkingPath = registry.walkingPaths.get(entity);
            if (walkingPath.path.size() > 0) {
                ivec2 next_pos = walkingPath.path[0];
                // std::cout << next_pos.x << ", " << next_pos.y << std::endl;

                // transform path position
                int transformed_path_x = next_pos.x * GRID_CELL_WIDTH_PX + GRID_CELL_WIDTH_PX / 2;
                int transformed_path_y = next_pos.y * GRID_CELL_HEIGHT_PX + GRID_CELL_HEIGHT_PX / 2;

                if (abs(motion.position.x - transformed_path_x) < 0.3f && abs(motion.position.y - transformed_path_y) < 0.3f) {
                    // remove the arrived path
                    walkingPath.path.erase(walkingPath.path.begin());

                } else {
                    vec2 direction = vec2(transformed_path_x, transformed_path_y) - motion.position;
                    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction.x /= length;
                        direction.y /= length;
                    }
                    
                    Enemy& enemy = registry.enemies.get(entity);
                    if (enemy.is_mod_affected) {
                        enemy.mod_effect_duration -= elapsed_ms;
                        if (enemy.mod_effect_duration <= 0) {
                            enemy.is_mod_affected = false;
                            enemy.speed = getEnemySpeed(enemy.type);
                        }
                    }

                    motion.velocity = direction * enemy.speed;

                    if (motion.velocity.x < 0) {
                        vec2 flip = {-1, 1};
                        motion.scale *= flip;
                    }
                }
            } else {
                // if no more walking path, remove entity from walkingPaths
                registry.walkingPaths.remove(entity);
            }
        } else if (registry.enemies.has(entity)) {
            Enemy& enemy = registry.enemies.get(entity);
            vec2 enemy_position = motion.position + CameraSystem::GetInstance()->position;

            Motion& ship_motion = registry.motions.get(registry.ships.entities[0]);
            vec2& ship_position = ship_motion.position;


            if (enemy_position != ship_position) {
                vec2 direction = ship_position - enemy_position;

                // unit vector for direction
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);

                if (enemy.range * GRID_CELL_WIDTH_PX < length) continue ; // ship not detected

                if (enemy.type == ENEMY_TYPE::SHOOTER && enemy.cooldown_ms <= 0) {
                    std::cout << entity.id() << "shoot" << std::endl;
                    createEnemyProjectile(enemy_position, ship_position);
                    enemy.cooldown_ms = ENEMY_PROJECTILE_COOLDOWN;
                    continue;
                }

                if (length > 0) {
                    direction.x /= length;
                    direction.y /= length;
                }

                
                Enemy& enemy = registry.enemies.get(entity);

                if (enemy.is_mod_affected) {
                    enemy.mod_effect_duration -= elapsed_ms;
                    if (enemy.mod_effect_duration <= 0) {
                        enemy.is_mod_affected = false;
                        enemy.speed = getEnemySpeed(enemy.type);
                    }
                }

                motion.velocity = direction * enemy.speed;

                if (motion.velocity.x < 0) {
                    vec2 flip = {-1, 1};
                    motion.scale *= flip;
                }
            }
        }
    }

    // check for collisions between all moving entities
    ComponentContainer<Motion>& motion_container = registry.motions;
    Base& base = registry.base.components[0];
    for (uint i = 0; i < motion_container.components.size(); i++) {
        Motion& motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];

        // note starting j at i+1 to compare all (i,j) pairs only once (and to not
        // compare with itself)
        for (uint j = i + 1; j < motion_container.components.size(); j++) {
            Entity entity_j = motion_container.entities[j];
            Motion& motion_j = motion_container.components[j];
            if ((registry.islands.has(entity_i) || registry.islands.has(entity_j)) &&
                (registry.ships.has(entity_i) || registry.ships.has(entity_j))) {
                // Poly collision only for islands.
                if (collidesPoly(entity_i, entity_j)) {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                }
            } else if ((registry.base.has(entity_i) || registry.base.has(entity_j)) &&
                       (registry.ships.has(entity_i) || registry.ships.has(entity_j))) {
                // Poly collision for base
                if (collidesPoly(entity_i, entity_j)) {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                    if (!base.ship_in_base) base.ship_in_base = true;
                    else base.drop_off_timer += elapsed_ms;
                } else {
                    if (base.ship_in_base) {
                        base.ship_in_base = false;
                        base.drop_off_timer = 0;
                    }        
                }
            } else if (registry.ships.has(entity_i) || registry.ships.has(entity_j)){
                // Handle SHIP collision.
                if (collidesSphericalShip(entity_i, entity_j))
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j); 
            } else if (collidesSpherical(motion_i, motion_j)) {
                // Every other collision.
                registry.collisions.emplace_with_duplicates(entity_i, entity_j);
            }
        }
    }
}
