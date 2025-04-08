#include "ai_system.hpp"
#include "camera_system.hpp"
#include <iostream>

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "pathing.hpp"
#include "physics_system.hpp"

#include <queue>

void AISystem::step(float elapsed_ms) {
    (void) elapsed_ms;
    for (const Entity& ship_entity : registry.ships.entities) {
        Motion& ship_motion = registry.motions.get(ship_entity);
        vec2& ship_position = ship_motion.position;

        // A* enemy path finding
        for (const Entity& enemy_entity : registry.enemies.entities) {
            Enemy& enemy = registry.enemies.get(enemy_entity);
            if (enemy.type == ENEMY_TYPE::BASIC_GUNNER && !registry.walkingPaths.has(enemy_entity)) {
                vec2 direction = ship_position - registry.motions.get(enemy_entity).position;
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);

                std::vector<ivec2> path;
                if (find_path(path, enemy_entity, ship_entity)) {
                    WalkingPath& walkingPath = registry.walkingPaths.emplace(enemy_entity);
                    walkingPath.path = path;
                } else {
                    // just remove the enemy if path is not found, otherwise it gets really laggy
                    registry.remove_all_components_of(enemy_entity);
                }
            }
        };
    }

    // enemy spawning and creation
    for (Entity entity : registry.enemySpawners.entities) {
        EnemySpawner& spawner = registry.enemySpawners.get(entity);
        if (spawner.cooldown_ms > 0) {
            spawner.cooldown_ms = (int)max(spawner.cooldown_ms * 1.0 - elapsed_ms, 0.0); // ensure cooldown isn't negative
        } else {
            Motion& spawner_motion = registry.motions.get(entity);
            vec2 spawner_position = spawner_motion.position;
            Motion& ship_motion = registry.motions.get(registry.ships.entities[0]);
            // make the ship position "move" instead of be stationary
            vec2 ship_position = ship_motion.position - CameraSystem::GetInstance()->position;
            vec2 direction = ship_position - spawner_position;
            float length = dot(direction, direction);
            int r_squared = (spawner.range * GRID_CELL_WIDTH_PX) *
                    (spawner.range * GRID_CELL_WIDTH_PX);

            // (shipRadius_x)^2 + (shipRadius_y)^2
            int ship_range = pow((ship_motion.scale.x / 2), 2) + pow((ship_motion.scale.y / 2), 2);

            // check if:
            // - ship is far enough away from ship
            // - within the spawner's range
            // - spawner is off cooldown
           
            if (ship_range <= length && length <= r_squared && spawner.cooldown_ms <= 0) {
                bool should_spawn = true;
                // don't respawn enemies if there is an existing enemy
                // too close to the spawner
                for (Entity enemy_entity : registry.enemies.entities) {
                    if (registry.motions.has(enemy_entity) &&
                        distance(registry.motions.get(enemy_entity).position,
                                 spawner_position) <=
                            sqrt(GRID_CELL_WIDTH_PX)) {
                        should_spawn = false;
                        break;
                    }
                    if (registry.enemies.get(enemy_entity).type == ENEMY_TYPE::DUMMY) {
                        should_spawn = false;
                        break;
                    }
                }
                if (should_spawn) {
                    createEnemy(entity);
                }
                spawner.cooldown_ms = ENEMY_BASE_SPAWN_CD_MS;  // reset spawn cooldown
            }
        }
    };
}

// enemy search for a path from itself to the ship, avoiding islands
// If a path is found, return true and store it in the vector of ivec2 tile coordinates.
// If a path is not found, return false.
bool AISystem::find_path(std::vector<ivec2> & path, Entity enemy_entity, Entity ship_entity)
{
    Motion& ship_motion = registry.motions.get(ship_entity);
    vec2 ship_position_with_camera = ship_motion.position - CameraSystem::GetInstance()->position;
    ivec2 ship_node_position = {(ship_position_with_camera.x - GRID_CELL_WIDTH_PX / 2) / GRID_CELL_WIDTH_PX, (ship_position_with_camera.y - GRID_CELL_HEIGHT_PX / 2) / GRID_CELL_HEIGHT_PX};
    
    Enemy& enemy = registry.enemies.get(enemy_entity);
    Motion& enemy_motion = registry.motions.get(enemy_entity);
    vec2 enemy_position = enemy_motion.position;
    ivec2 enemy_node_position = {(enemy_position.x - GRID_CELL_WIDTH_PX / 2) / GRID_CELL_WIDTH_PX, (enemy_position.y - GRID_CELL_HEIGHT_PX / 2) / GRID_CELL_HEIGHT_PX};

    Node enemy_node = Node(enemy_node_position, enemy_node_position, 0, 0);
    Node ship_node = Node(ship_node_position, ship_node_position, 0, 0);

    enemy_node.H = get_distance(enemy_node, ship_node);
    enemy_node.F = enemy_node.get_score();
    ship_node.G = get_distance(ship_node, enemy_node);
    ship_node.F = ship_node.get_score();

    auto comparator = [](const Node& a, const Node& b) {
        if (a.F == b.F) {
            return a.H > b.H;  // Lower H has higher priority if equal F
        }
        // Lower F has higher priority
        return a.F > b.F;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(comparator)> open_nodes(comparator);
    std::vector<Node> closed_nodes;
    open_nodes.push(enemy_node);

    // use A*
    // return true when a path is found and return the path via the &path vector
    while (open_nodes.size() > 0) {
        Node current_node = open_nodes.top();
        open_nodes.pop();
        closed_nodes.push_back(current_node);
    
        if (current_node == ship_node) {
            std::cout << "path found" << std::endl;
            path.clear();
            path = retrace_path(closed_nodes, enemy_node, ship_node);
            for (ivec2 p : path) {
                vec2 fill_pos = {p.x * GRID_CELL_WIDTH_PX + GRID_CELL_WIDTH_PX / 2, p.y * GRID_CELL_HEIGHT_PX + GRID_CELL_HEIGHT_PX / 2};
                // createFilledTile(fill_pos, {56, 56});
            }
            return true;
        }
    
        for (ivec2 neighbour : get_walkable_neighbours(current_node)) {
            Node neighbour_node = Node({neighbour.x, neighbour.y}, current_node.position, 0, 0);

            // check if node is already closed
            if (std::find(closed_nodes.begin(), closed_nodes.end(), neighbour_node) != closed_nodes.end()) continue;

            uint new_neighbour_G = current_node.G + get_distance(current_node, neighbour_node);

            // check if node is already open
            bool is_open = false;
            auto temp = open_nodes;
            while (!temp.empty()) {
                if (temp.top().position == neighbour_node.position) {
                    is_open = true;
                    break;
                }
                temp.pop();
            }

            if (new_neighbour_G < neighbour_node.G || !is_open) {

                neighbour_node.G = new_neighbour_G;
                neighbour_node.H = get_distance(neighbour_node, ship_node);
                neighbour_node.F = neighbour_node.G + neighbour_node.H;
                neighbour_node.parent = current_node.position;

                if (!is_open) {
                    open_nodes.push(neighbour_node);
                }
            }
        }
    }

	// we did not find a valid path...
    // std::cout << "path not found" << std::endl;
	return false;
}

uint AISystem::get_distance(Node node1, Node node2) {
	uint dstX = abs(node1.position.x - node2.position.x);
	uint dstY = abs(node1.position.y - node2.position.y);

	if (dstX > dstY)
		return 14 * dstY + 10 * (dstX - dstY);

	return 14 * dstX + 10 * (dstY - dstX);
}

std::vector<ivec2> AISystem::get_walkable_neighbours(Node node) {
    std::vector<ivec2> neighbours;
    std::vector<ivec2> possible_neighbours = {
        {node.position.x - 1, node.position.y - 1},     // top left
        {node.position.x, node.position.y - 1},         // top middle
        {node.position.x + 1, node.position.y - 1},     // top botom
        {node.position.x - 1, node.position.y},         // middle left
        {node.position.x + 1, node.position.y},         // middle right
        {node.position.x - 1, node.position.y + 1},     // bottom left
        {node.position.x, node.position.y + 1},         // bottom middle
        {node.position.x + 1, node.position.y + 1}     // bottom right
    };


    for (ivec2 neighbour : possible_neighbours) {
        bool should_add = true;
        for (Entity entity : registry.islands.entities) {
            if (PhysicsSystem::collidesPolyVec(entity, neighbour)) {
                should_add = false;
                break;
            }
        }
        if (!should_add) continue;
        neighbours.push_back(neighbour);
    }

    return neighbours;
}

std::vector<ivec2> AISystem::retrace_path(std::vector<Node> visited_nodes, Node start_node, Node exit_node) {
	std::vector<ivec2> path;
	Node current_node = exit_node;

	while (current_node.position != start_node.position) {
		// add current position to path
		if (current_node.parent != exit_node.position) {
			path.push_back(current_node.position);
		}

		// find parent node
		auto it = std::find_if(visited_nodes.begin(), visited_nodes.end(), [&](const Node& node) {
			return node.position == current_node.parent;
		});

		if (it != visited_nodes.end()) {
			current_node = *it; // update current_node to the parent node
		} else {
			break;
		}

	}

	path.push_back(start_node.position);

	std::reverse(path.begin(), path.end());

	return path;
}
