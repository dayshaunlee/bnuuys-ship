#include "saveload_system.hpp"
#include "common.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(tile_position, x, y);

std::string SaveLoadSystem::moduleTypeToString(MODULE_TYPES type) {
  switch (type) {
    case MODULE_TYPES::EMPTY: return "EMPTY";
    case MODULE_TYPES::PLATFORM: return "PLATFORM";
    case MODULE_TYPES::STEERING_WHEEL: return "STEERING_WHEEL";
    case MODULE_TYPES::SIMPLE_CANNON: return "SIMPLE_CANNON";
    case MODULE_TYPES::LASER_WEAPON: return "LASER_WEAPON";
    case MODULE_TYPES::HELPER_BUNNY: return "HELPER_BUNNY";
    case MODULE_TYPES::BUBBLE_MOD: return "BUBBLE_MOD";
    case MODULE_TYPES::HEAL: return "HEAL";

    default: return "UNKNOWN";
  }
}


MODULE_TYPES SaveLoadSystem::moduleTypeFromString(const std::string& str) {
  if (str == "EMPTY") return MODULE_TYPES::EMPTY;
  if (str == "PLATFORM") return MODULE_TYPES::PLATFORM;
  if (str == "STEERING_WHEEL") return MODULE_TYPES::STEERING_WHEEL;
  if (str == "SIMPLE_CANNON") return MODULE_TYPES::SIMPLE_CANNON;
  if (str == "LASER_WEAPON") return MODULE_TYPES::LASER_WEAPON;
  if (str == "HELPER_BUNNY") return MODULE_TYPES::HELPER_BUNNY;
  if (str == "BUBBLE_MOD") return MODULE_TYPES::BUBBLE_MOD;
  if (str == "HEAL") return MODULE_TYPES::HEAL;

  throw std::invalid_argument("Unknown MODULE_TYPE string: " + str);
}

void SaveLoadSystem::toJson(json& j, const GameData& data) {
  j["playerName"] = data.playerName;
  j["levelName"] = data.levelName;

  j["ship_health"] = data.ship_health;
  j["ship_maxHealth"] = data.ship_maxHealth;
  j["ship_is_expanded"] = data.ship_is_expanded;

  // Serialize used_modules
  json usedModulesJson = json::array();
  for (const auto& row : data.used_modules) {
      json rowJson = json::array();
      for (const auto& module : row) {
          rowJson.push_back(moduleTypeToString(module));
      }
      usedModulesJson.push_back(rowJson);
  }
  j["used_modules"] = usedModulesJson;

  // Serialize unused_modules
  json unusedModulesJson;
  for (const auto& [type, count] : data.unused_modules) {
      unusedModulesJson[moduleTypeToString(type)] = count;
  }
  j["unused_modules"] = unusedModulesJson;
}

void SaveLoadSystem::fromJson(const json& j, GameData& data) {
  data.playerName = j.at("playerName").get<std::string>();
  data.levelName = j.at("levelName").get<std::string>();

  data.ship_health = j.at("ship_health").get<int>();
  data.ship_maxHealth = j.at("ship_maxHealth").get<int>();
  data.ship_is_expanded = j.at("ship_is_expanded").get<int>();

  // Deserialize used_modules
  const auto& usedModulesJson = j.at("used_modules");
  data.used_modules.clear();
  for (const auto& rowJson : usedModulesJson) {
      std::vector<MODULE_TYPES> row;
      for (const auto& moduleStr : rowJson) {
          row.push_back(moduleTypeFromString(moduleStr.get<std::string>()));
      }
      data.used_modules.push_back(row);
  }

  // Deserialize unused_modules
  const auto& unusedModulesJson = j.at("unused_modules");
  data.unused_modules.clear();
  for (auto it = unusedModulesJson.begin(); it != unusedModulesJson.end(); ++it) {
      MODULE_TYPES type = moduleTypeFromString(it.key());
      int count = it.value().get<int>();
      data.unused_modules[type] = count;
  }

  return;

}

GameData SaveLoadSystem::createGameData(std::string _playerName, std::string _levelName, Ship ship) {
  GameData data;
  data.playerName = _playerName;
  data.levelName = _levelName;
  data.ship_health = ship.health;
  data.ship_maxHealth = ship.maxHealth;
  data.ship_is_expanded = ship.is_expanded;
  data.used_modules = ship.ship_modules;
  data.unused_modules = ship.available_modules;

  return data;
}

void SaveLoadSystem::saveGame(const GameData& data, const std::string& fileName) {
  json j;
  toJson(j, data);

  std::string path;
  path = data_path() + "/" + fileName;

  std::ofstream file(path);
  if (!file.is_open()) {
      std::cerr << "Failed to open file: " << fileName << std::endl;
      return;
  }
  file << j.dump(4);

  SaveLoadSystem::getInstance().loadedGameData = data;
  SaveLoadSystem::getInstance().hasLoadedData = true;

}

bool SaveLoadSystem::loadGame(GameData& data, const std::string& fileName) {
  std::string path;
    path = data_path() + "/" + fileName;

  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << fileName << std::endl;
    return false;
}

  nlohmann::json j;
  file >> j;
  fromJson(j, data);

  return true;
}
