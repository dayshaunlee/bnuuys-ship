#pragma once

#include "common.hpp"

// A2: simple A-star node
struct Node {
	uint G, H, F;
	ivec2 position;    // tile-based
	ivec2 parent;      // tile-based

	Node(ivec2 position_, ivec2 parent_, uint g_, uint h_) : position(position_), parent(parent_), G(g_), H(h_) { F = G + H; };

	uint get_score() { return G + H; }
	
	bool operator==(const Node& rhs) { return this->position == rhs.position; }

	bool operator==(const ivec2& pos) { return this->position == pos; }
};

