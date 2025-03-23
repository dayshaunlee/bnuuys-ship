#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

#include <../ext/json/json.hpp>

using json = nlohmann::json;

// save and load system

struct tile_position {
  int x;
  int y;
};

struct GameData {
  std::string playerName;
  std::string levelName;
  float ship_health;
  float ship_maxHealth;
  bool ship_is_expanded;
  std::vector<std::vector<MODULE_TYPES>> used_modules;
  std::unordered_map<MODULE_TYPES, uint> unused_modules;
};

class SaveLoadSystem {
   public:

    static SaveLoadSystem& getInstance() {
      static SaveLoadSystem instance;
      return instance;
    }

    GameData createGameData(std::string _playerName, std::string _levelName, Ship ship);
    void saveGame(const GameData& data, const std::string& fileName);
    bool loadGame(GameData& data, const std::string& fileName);

    GameData loadedGameData;
    bool hasLoadedData = false;

   private:
    std::string moduleTypeToString(MODULE_TYPES type);
    MODULE_TYPES moduleTypeFromString(const std::string& str);

    void toJson(json& j, const GameData& data);
    void fromJson(const json& j, GameData& data);

};