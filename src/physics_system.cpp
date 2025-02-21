// internal
#include "physics_system.hpp"

#include <iostream>

#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the
// entity
vec2 get_bounding_box(const Motion& motion) {
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

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

std::vector<tson::Vector2i> get_poly_from_motion(const Motion& motion) {
    std::vector<tson::Vector2i> polygon;
    int posX = motion.position.x;
    int posY = motion.position.y;
    int halfWidth = motion.scale.x / 2;
    int halfHeight = motion.scale.y / 2;

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

// This is a SUPER APPROXIMATE check that puts a circle around the bounding
// boxes and sees if the center point of either object is inside the other's
// bounding-box-circle. You can surely implement a more accurate detection
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

// Polygon - Polygon collision
bool collidesPoly(const Entity e1, const Entity e2) {
    if ((registry.islands.has(e1) && registry.ships.has(e2)) || (registry.islands.has(e2) && registry.ships.has(e1))) {
        Motion& e1_mot = registry.motions.get(e1);
        Motion& e2_mot = registry.motions.get(e2);
        std::vector<tson::Vector2i> islandPolygon;
        std::vector<tson::Vector2i> shipPolygon;
        if (registry.islands.has(e1)) {  // e1 is the Island, e2 is the Ship
            islandPolygon = registry.islands.get(e1).polygon;
            shipPolygon = get_poly_from_motion(e2_mot);
            for (auto& p : islandPolygon) {  // account for camera affecting position
                p.x += e1_mot.position.x;
                p.y += e1_mot.position.y;
            }
        } else {  // e2 is the Island, e1 is the Ship
            islandPolygon = registry.islands.get(e2).polygon;
            shipPolygon = get_poly_from_motion(e1_mot);
            for (auto& p : islandPolygon) {  // account for camera affecting position
                p.x += e2_mot.position.x;
                p.y += e2_mot.position.y;
            }
        }
        return polyPoly(islandPolygon, shipPolygon);
    }
    return false;
}

void PhysicsSystem::step(float elapsed_ms) {
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
        }
    }

    // check for collisions between all moving entities
    ComponentContainer<Motion>& motion_container = registry.motions;
    for (uint i = 0; i < motion_container.components.size(); i++) {
        Motion& motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];

        // note starting j at i+1 to compare all (i,j) pairs only once (and to not
        // compare with itself)
        for (uint j = i + 1; j < motion_container.components.size(); j++) {
            Entity entity_j = motion_container.entities[j];
            Motion& motion_j = motion_container.components[j];
            if (collidesPoly(entity_i, entity_j)) {
                registry.collisions.emplace_with_duplicates(entity_i, entity_j);
            }
        }
    }
}