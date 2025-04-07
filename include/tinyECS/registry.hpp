#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry {
    // callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface*> registry_list;

   public:
    // Manually created list of all components this game has
    ComponentContainer<RenderRequest> renderRequests;
    ComponentContainer<RenderLayer> renderLayers;
    ComponentContainer<GridLine> gridLines;
    ComponentContainer<Overlay> overlays;
    ComponentContainer<Spotlight> spotlights;
    ComponentContainer<ScreenState> screenStates;
    ComponentContainer<vec3> colors;

    ComponentContainer<Player> players;
    ComponentContainer<PlayerAnimation> playerAnimations;

    ComponentContainer<Ship> ships;

    ComponentContainer<Motion> motions;
    ComponentContainer<Collision> collisions;
    ComponentContainer<Sound> sounds;

    // backgroundObject component for camera
    ComponentContainer<BackgroundObject> backgroundObjects;
    ComponentContainer<Camera> cameras;
    ComponentContainer<Enemy> enemies;
    ComponentContainer<EnemySpawner> enemySpawners;

    ComponentContainer<Island> islands;
    ComponentContainer<Base> base;

    ComponentContainer<SteeringWheel> steeringWheels;
    ComponentContainer<SimpleCannon> simpleCannons;
    
    ComponentContainer<CannonModifier> cannonModifiers;

    ComponentContainer<LaserWeapon> laserWeapons;
    ComponentContainer<LaserBeam> laserBeams;

    ComponentContainer<Heal> healModules;

    ComponentContainer<PlayerProjectile> playerProjectiles;
    ComponentContainer<EnemyProjectile> enemyProjectiles;
    ComponentContainer<Bunny> bunnies;

    ComponentContainer<WalkingPath> walkingPaths;
    ComponentContainer<FilledTile> filledTiles;

    ComponentContainer<Disaster> disasters;
    ComponentContainer<HelperBunnyIcon> helperBunnyIcons;

    ComponentContainer<ParticleEmitter> particleEmitters;
    

    // constructor that adds all containers for looping over them
    ECSRegistry() {
        registry_list.push_back(&renderRequests);
        registry_list.push_back(&renderLayers);
        registry_list.push_back(&gridLines);
        registry_list.push_back(&overlays);
        registry_list.push_back(&spotlights);
        registry_list.push_back(&screenStates);
        registry_list.push_back(&colors);

        registry_list.push_back(&players);
        registry_list.push_back(&playerAnimations);

        registry_list.push_back(&ships);
        
        registry_list.push_back(&motions);
        registry_list.push_back(&collisions);
        registry_list.push_back(&sounds);
        
        registry_list.push_back(&backgroundObjects);
        registry_list.push_back(&cameras);

        registry_list.push_back(&islands);
        registry_list.push_back(&base);

        registry_list.push_back(&steeringWheels);
        registry_list.push_back(&simpleCannons);

        registry_list.push_back(&cannonModifiers);

        registry_list.push_back(&laserWeapons);
        registry_list.push_back(&laserBeams);

        registry_list.push_back(&healModules);
        
        registry_list.push_back(&enemies);
        registry_list.push_back(&enemySpawners);

        registry_list.push_back(&bunnies);
        registry_list.push_back(&walkingPaths);
        registry_list.push_back(&filledTiles);

        registry_list.push_back(&disasters);
        registry_list.push_back(&helperBunnyIcons);

        registry_list.push_back(&particleEmitters);
    }

    void clear_all_components() {
        for (ContainerInterface* reg : registry_list) reg->clear();
    }

    void list_all_components() {
        printf("Debug info on all registry entries:\n");
        for (ContainerInterface* reg : registry_list)
            if (reg->size() > 0) printf("%4d components of type %s\n", (int) reg->size(), typeid(*reg).name());
    }

    void list_all_components_of(Entity e) {
        printf("Debug info on components of entity %u:\n", (unsigned int) e);
        for (ContainerInterface* reg : registry_list)
            if (reg->has(e)) printf("type %s\n", typeid(*reg).name());
    }

    void remove_all_components_of(Entity e) {
        for (ContainerInterface* reg : registry_list) reg->remove(e);
    }
};

extern ECSRegistry registry;
