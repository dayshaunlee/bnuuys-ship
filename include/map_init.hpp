#pragma once
#include "../ext/tileson/tileson.hpp"

// Map
void loadMap(const std::string& filename);

std::string objectTypeStr(tson::ObjectType objType);

void debugMap(std::unique_ptr<tson::Map>& map);