#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "pathing.hpp"

class AISystem {
   public:
    void step(float elapsed_ms);
    bool find_path(std::vector<ivec2> & path, Entity enemy_entity, Entity ship_entity);

   private: 
    uint get_distance(Node node1, Node node2);
    std::vector<ivec2> get_walkable_neighbours(Node node);
    std::vector<ivec2> retrace_path(std::vector<Node> visited_nodes, Node start_node, Node exit_node);
};
