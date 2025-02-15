#pragma once
#include "../ext/tileson/tileson.hpp"
#include <vector>

// Loads map information and objects and creates relevant Entities, returns size of map
// NOTE: rendering is not handled by the map -- instead just draw background with the map .png
/* TODO (X=COMPLETE):
*	- loop through all layers. if it is of type 'objectgroup' then:
*		- loop through "island" layer and:
*			- if it is of class 'island' -> create Entities with Island and Motion
*			- if it is of class 'base' -> create Entity with Base and Motion 
*		- loop through "spawnpoints" layer and:
*			- if it is of class 'enemy' -> create Entities with Enemy and Motion
*			- if it is of class 'player' -> update Player position (if exists)
*	X return map size as tson::Vector2<int>
*/
tson::Vector2<int> loadMap(const std::string& filename);

std::string objectTypeStr(tson::ObjectType objType);

void debugMap(std::unique_ptr<tson::Map>& map);