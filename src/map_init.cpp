#include "world_init.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
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

void loadMap(const std::string& name) {

    fs::path path = fs::path(map_path(name));
    tson::Tileson t;
    std::unique_ptr<tson::Map> map = t.parse(path);

    if (map->getStatus() == tson::ParseStatus::OK) {
        debugMap(map);
    } else  // Error occured
    {
        std::cout << map->getStatusMessage();
    }
}