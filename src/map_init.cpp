#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/entity.hpp"
#include "../ext/tileson/tileson.hpp"
#include <iostream>
#include <filesystem>

std::string objectTypeStr(tson::ObjectType objType) {
    switch (objType) {
        case tson::ObjectType::Polygon:
            return "polygon";
            break;
        case tson::ObjectType::Point:
            return "point";
            break;
        case tson::ObjectType::Ellipse:
            return "ellipse";
            break;
        case tson::ObjectType::Rectangle:
            return "rectangle";
            break;
        case tson::ObjectType::Polyline:
            return "polyline";
            break;
        case tson::ObjectType::Template:
            return "template";
            break;
        case tson::ObjectType::Text:
            return "text";
            break;
        default:
            return "undefined";
    }
};

Motion getPolygonMotion(const std::vector<tson::Vector2i>& polygon) {
    if (polygon.empty()) {
        return {{0, 0}, 0, {0, 0}, {10, 10}};
    }

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();

    for (const auto& p : polygon) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    vec2 center((minX + maxX) / 2, (minY + maxY) / 2);
    vec2 size(maxX - minX, maxY - minY);

    // Motion: position, angle, velocity, scale
    return {center, 0, {0, 0}, size};
}

void debugMap(std::unique_ptr<tson::Map>& map) {
    std::cout << "Map size: x=" << map->getSize().x << " y=" << map->getSize().y << std::endl;
    std::cout << "Render order: " << map->getRenderOrder() << std::endl;
    std::cout << "Tile size (px): x=" << map->getTileSize().x << " y=" << map->getTileSize().y << std::endl;
    std::cout << "\nTilesets used:" << std::endl;
    for (tson::Tileset tileset : map->getTilesets()) {
        std::cout << "Tileset with name '" << tileset.getName() << "' loaded from " << tileset.getImage() << std::endl;
        std::cout << "Tile count: " << tileset.getTileCount() << std::endl;
        // gid is 1-indexed, access tson::Tile* tiles with map->getTileMap()[gid]
        std::cout << "Global tile id of first tile: " << tileset.getFirstgid() << std::endl;
    }
    std::cout << "\nMap layers:" << std::endl;
    for (tson::Layer layer : map->getLayers()) {
        std::cout << "Layer with name '" << layer.getName() << "' of type " << layer.getTypeStr() << std::endl;
        const int layerSize = layer.getData().size();
        if (layerSize > 0)
            std::cout << "Tile count: " << layerSize << std::endl;
        else {
            std::cout << "Object count: " << layer.getObjects().size() << std::endl;
            std::cout << "\nObjects: " << layer.getObjects().size() << std::endl;
            for (tson::Object object : layer.getObjects()) {
                std::cout << "Object class: " << object.getType() << " of type "
                          << objectTypeStr(object.getObjectType()) << " at x=" << object.getPosition().x
                          << " y=" << object.getPosition().y << std::endl;
                if (object.getObjectType() == tson::ObjectType::Polygon) {
                    std::cout << "\nPolygon points: " << std::endl;
                    for (tson::Vector2<int> vertex : object.getPolygons()) {
                        std::cout << "x=" << vertex.x << " y=" << vertex.y << std::endl;
                    }
                }
                std::string typeStr;
            }
        }
    }
};

/* LOAD MAP
 *	- loop through all layers. if it is of type 'objectgroup' then:
 *		- loop through "island" layer and:
 *			- if it is of class 'island' -> create Entities with Island and Motion
 *			- if it is of class 'base' -> create Entity with Base and Motion
 *		- loop through "spawnpoints" layer and:
 *			- if it is of class 'enemy' -> create Entities with Enemy and Motion
 *			- if it is of class 'player' -> update Player position (if exists)
 *	- return map size as tson::Vector2<int>
 */
std::pair<tson::Vector2i, tson::Vector2i> loadMap(const std::string& name) {
    // delete stuff from previous maps
    registry.enemies.clear();
    registry.islands.clear();
    registry.base.clear();

    // load project to retrieve custom properties
    tson::Project project = tson::Project{fs::path(map_path("bnunny_savers.tiled-project"))};

    // load map from file
    fs::path path = fs::path(map_path(name));
    tson::Tileson t{&project};
    std::unique_ptr<tson::Map> map = t.parse(path);

    if (map->getStatus() == tson::ParseStatus::OK) {
        // debugMap(map);
        float scaling_factor_x = GRID_CELL_WIDTH_PX / (float) map->getTileSize().x;
        float scaling_factor_y = GRID_CELL_HEIGHT_PX / (float) map->getTileSize().y;
        tson::Vector2i offset(0, 0);

        tson::Layer* spawns_layer = map->getLayer("spawnpoints");
        assert(spawns_layer != nullptr);                                  // ensure layer exists
        assert(spawns_layer->getType() == tson::LayerType::ObjectGroup);  // ensure it is object layer

        for (auto& obj : spawns_layer->getObjects()) {
            if (obj.getClassType() == "player") {
                if (registry.players.size() == 1) {
                    Entity e = registry.players.entities[0];
                    Motion& mot = registry.motions.get(e);
                    offset.x = (WINDOW_WIDTH_PX / 2) - (obj.getPosition().x * scaling_factor_x);
                    offset.y = (WINDOW_HEIGHT_PX / 2) - (obj.getPosition().y * scaling_factor_y);
                    mot.position = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2};
                }
            } else if (obj.getClassType() == "enemy") {
                Entity e = Entity();
                EnemySpawner& ene = registry.enemySpawners.emplace(e);
                tson::EnumValue ene_type = obj.get<tson::EnumValue>("enemy_type_enum");
                ene.type = static_cast<ENEMY_TYPE>(ene_type.getValue());
                ene.home_island = obj.get<uint32_t>("home_island");
                Motion& mot = registry.motions.emplace(e);
                mot.position = {obj.getPosition().x * scaling_factor_x + offset.x,
                                obj.getPosition().y * scaling_factor_y + offset.y};
                mot.scale = {obj.getSize().x * scaling_factor_x, obj.getSize().y * scaling_factor_y};
                registry.backgroundObjects.emplace(e);
            } else if (obj.getClassType() == "bunny") {
                Entity e = Entity();
                Bunny& bun = registry.bunnies.emplace(e);
                Motion& mot = registry.motions.emplace(e);
                mot.position = {obj.getPosition().x * scaling_factor_x + offset.x,
                                obj.getPosition().y * scaling_factor_y + offset.y};
                mot.scale = {obj.getSize().x * scaling_factor_x, obj.getSize().y * scaling_factor_y};
            } else if (obj.getClassType() == "disaster") {
                Entity e = Entity();
                Disaster& dis = registry.disasters.emplace(e);
                tson::EnumValue dis_type = obj.get<tson::EnumValue>("disaster_type");
                dis.type = static_cast<DISASTER_TYPE>(dis_type.getValue());
                Motion& mot = registry.motions.emplace(e);
                mot.position = {obj.getPosition().x * scaling_factor_x + offset.x,
                                obj.getPosition().y * scaling_factor_y + offset.y};
                mot.scale = {obj.getSize().x * scaling_factor_x, obj.getSize().y * scaling_factor_y};
                registry.backgroundObjects.emplace(e);
            }
        }

        tson::Layer* islands_layer = map->getLayer("islands");
        assert(islands_layer != nullptr);                                  // ensure layer exists
        assert(islands_layer->getType() == tson::LayerType::ObjectGroup);  // ensure it is object layer
        for (auto& obj : islands_layer->getObjects()) {
            if (obj.getClassType() == "island") {
                Entity e = Entity();
                Motion& mot = registry.motions.emplace(e);
                mot.position = {obj.getPosition().x * scaling_factor_x + offset.x,
                                obj.getPosition().y * scaling_factor_y + offset.y};
                mot.scale = {obj.getSize().x * scaling_factor_x, obj.getSize().y * scaling_factor_y};
                Island& isl = registry.islands.emplace(e);
                isl.polygon = obj.getPolygons();  // first point is always (0, 0)
                Motion centeredMot1 = getPolygonMotion(isl.polygon);
                for (auto& p : isl.polygon) {     // scale up polygons too
                    p.x *= scaling_factor_x;
                    p.y *= scaling_factor_y;
                }
                Motion centeredMot = getPolygonMotion(isl.polygon);
                mot.position += centeredMot.position;
                mot.scale = centeredMot.scale;
                for (auto& p : isl.polygon) {  // recenter polygon
                    p.x -= centeredMot.position.x;
                    p.y -= centeredMot.position.y;
                }
                registry.backgroundObjects.emplace(e);
            } else if (obj.getClassType() == "base") {
                Entity e = Entity();
                Base& bas = registry.base.emplace(e);
                bas.polygon = obj.getPolygons();
                for (auto& p : bas.polygon) {  // scale up polygons too
                    p.x *= scaling_factor_x;
                    p.y *= scaling_factor_y;
                }
                Motion& mot = registry.motions.emplace(e);
                mot.position = {obj.getPosition().x * scaling_factor_x + offset.x,
                                obj.getPosition().y * scaling_factor_y + offset.y};
                mot.scale = {obj.getSize().x * scaling_factor_x, obj.getSize().y * scaling_factor_y};
                Motion centeredMot = getPolygonMotion(bas.polygon);
                mot.position += centeredMot.position;
                mot.scale = centeredMot.scale;
                for (auto& p : bas.polygon) {  // recenter polygon
                    p.x -= centeredMot.position.x;
                    p.y -= centeredMot.position.y;
                }
                registry.backgroundObjects.emplace(e);
            }
        }
        tson::Vector2i map_size(int(map->getSize().x * map->getTileSize().x * scaling_factor_x),
                                int(map->getSize().y * map->getTileSize().y * scaling_factor_y));
        return std::make_pair(map_size, offset);
    } else  // Error occured
    {
        std::cout << map->getStatusMessage();
        return std::make_pair(tson::Vector2i(0, 0), tson::Vector2i(0, 0));
    }
}