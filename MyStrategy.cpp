#include "MyStrategy.hpp"
#include <exception>
#include <iostream>
#include <cstdlib>
#include <algorithm>


int dist(const Entity& e1, const Entity& e2) {
    return std::abs(e1.position.x - e2.position.x) + std::abs(e1.position.y - e2.position.y);
}

bool eqPlayerTeam(int p1, int p2, const PlayerView& playerView) {
    if (p1 == -1 || p2 == -1) {
        return false;
    }
    if ((p1 == playerView.myId && p2 == playerView.myId) || (p1 != playerView.myId && p2 != playerView.myId)) {
        return true;
    }
    return false;
}

MyStrategy::MyStrategy() {}

Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface) {
    int anyEnemyId;
    for (const Player& player : playerView.players) {
        if (player.id != playerView.myId) {
            anyEnemyId = player.id;
        }
    }

    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            world[i][j] = -1;
        }
    }
    for (const auto& entity : playerView.entities) {
        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);
        for (int i = entity.position.x; i < entity.position.x + properties.size; ++i) {
            for (int j = entity.position.y; j < entity.position.y + properties.size; ++j) {
                world[i][j] = entity.id;
            }
        }
    }
    
//    myToMyMapping = calculateDistances(playerView, playerView.myId, playerView.myId);
    myToEnemyMapping = calculateDistances(playerView, playerView.myId, anyEnemyId);
//    enemyToMyMapping = calculateDistances(playerView, anyEnemyId, playerView.myId);
//    enemyToEnemyMapping = calculateDistances(playerView, anyEnemyId, anyEnemyId);

    std::unordered_map<int, EntityAction> actions;

    getBuildUnitActions(playerView, actions);
    getFarmerActions(playerView, actions);
    getWarriorActions(playerView, actions);

    return actions;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface) {
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}

void MyStrategy::getBuildUnitActions(const PlayerView& playerView, Actions& actions) {
    const auto& myPlayer = playerView.playersById.at(playerView.myId);
    if (myPlayer.resource < 10 || playerView.getFood() == 0) {
        return;
    }

    int buildersCount = playerView.GetMyEntities(BUILDER_UNIT).size();
    int rangedCount = playerView.GetMyEntities(RANGED_UNIT).size();

    if (playerView.GetMyEntities(BUILDER_BASE).empty() || playerView.GetMyEntities(RANGED_BASE).empty()) {
        return;
    }
    const Entity& builderBase = playerView.GetMyEntities(BUILDER_BASE)[0];
    const Entity& rangedBase = playerView.GetMyEntities(RANGED_BASE)[0];
    if (buildersCount < 15) {
        actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, false);
    } else if (buildersCount < 80) {
        if (rangedCount < buildersCount - 12) {
            actions[rangedBase.id] = createBuildUnitAction(rangedBase, RANGED_UNIT, true);
        } else {
            actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, true);
        }
    } else {
        actions[rangedBase.id] = createBuildUnitAction(rangedBase, RANGED_UNIT, true);
    }
}

void MyStrategy::getFarmerActions(const PlayerView& playerView, Actions& actions) {
    std::unordered_set<EntityType> BUILDING_TYPES = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, TURRET, WALL};
    const auto& units = playerView.GetMyEntities(BUILDER_UNIT);

    std::unordered_set<int> brokenBuildings;
    for (const Entity& base : playerView.entities) {
        if (BUILDING_TYPES.count(base.entityType)
                && base.health < playerView.entityProperties.at(base.entityType).maxHealth) {
            brokenBuildings.insert(base.id);
        }
    }
    const auto& myPlayer = playerView.playersById.at(playerView.myId);
    std::unordered_set<int> houseBuilders;
    std::vector<Entity> potentialHouseBuilders;
    if (playerView.getFood() < 5 && myPlayer.resource >= 50) {
        for (const Entity& unit : units) {
            bool x_mod = (unit.position.x % 5 == 0 || unit.position.x % 5 == 4);
            bool y_mod = (unit.position.y % 5 == 0 || unit.position.y % 5 == 4);
            if ((x_mod && !y_mod) || (!x_mod && y_mod)) {
                if (isEmptyForHouse(unit.position.x / 5 * 5 + 2, unit.position.y / 5 * 5 + 2)) {
                    potentialHouseBuilders.emplace_back(unit);
                }
            }
        }
    }
    if (!potentialHouseBuilders.empty()) {
        std::sort(potentialHouseBuilders.begin(), potentialHouseBuilders.end(), [] (const Entity& unit1, const Entity& unit2) {
            return unit1.position.x + unit1.position.y < unit2.position.x + unit2.position.y;
        });
        const auto& unit = potentialHouseBuilders[0];
        houseBuilders.insert(unit.id);
        actions[unit.id] = BuildAction(HOUSE, {unit.position.x / 5 * 5 + 1, unit.position.y / 5 * 5 + 1});
    }

    for (const Entity& unit : units) {
//        if (!builderMeta.count(unit.id)) {
//            int minResourceDist = 1000;
//            Vec2Int minResourcePosition;
//            for (const auto& resource : playerView.entities) {
//                if (resource.entityType == RESOURCE) {
//                    int resourceDist = dist(unit, resource);
//                    if (resourceDist < minResourceDist) {
//                        minResourceDist = resourceDist;
//                        minResourcePosition = unit.position;
//                    }
//                }
//            }
//            if (minResourceDist < 30) {
//                builderMeta[unit.id] = BuilderMeta(BuilderState::FARM);
//                actions[unit.id] = AttackAction({1000, {RESOURCE}});
//            } else {
//                builderMeta[unit.id] = BuilderMeta({BuilderState::MOVE_TO_FARM, minResourcePosition});
//                actions[unit.id] = MoveAction(minResourcePosition, false, false);
//            }
//            continue;
//        }
        if (houseBuilders.count(unit.id)) {
            continue;
        }

//        const BuilderMeta& meta = builderMeta.at(unit.id);

        if (unit.position.x != 79 && brokenBuildings.count(world[unit.position.x + 1][unit.position.y])) {
//            builderMeta[unit.id] = BuilderMeta(BuilderState::REPAIR);
            actions[unit.id] = RepairAction(world[unit.position.x + 1][unit.position.y]);
            continue;
        }
        if (unit.position.x != 0 && brokenBuildings.count(world[unit.position.x - 1][unit.position.y])) {
//            builderMeta[unit.id] = BuilderMeta(BuilderState::REPAIR);
            actions[unit.id] = RepairAction(world[unit.position.x - 1][unit.position.y]);
            continue;
        }
        if (unit.position.y != 79 && brokenBuildings.count(world[unit.position.x][unit.position.y + 1])) {
//            builderMeta[unit.id] = BuilderMeta(BuilderState::REPAIR);
            actions[unit.id] = RepairAction(world[unit.position.x][unit.position.y + 1]);
            continue;
        }
        if (unit.position.y != 0 && brokenBuildings.count(world[unit.position.x][unit.position.y - 1])) {
//            builderMeta[unit.id] = BuilderMeta(BuilderState::REPAIR);
            actions[unit.id] = RepairAction(world[unit.position.x][unit.position.y - 1]);
            continue;
        }

//        if (meta.state == BuilderState::FARM) {
//            actions[unit.id] = AttackAction({1000, {RESOURCE}});
//        } else if (meta.state == BuilderState::MOVE_TO_FARM) {
//            if (unit.position == *meta.target) {
//                builderMeta[unit.id] = BuilderMeta(BuilderState::FARM);
//                actions[unit.id] = AttackAction({1000, {RESOURCE}});
//            }
//        } else if (meta.state == BuilderState::REPAIR) {
            actions[unit.id] = AttackAction({1000, {RESOURCE}});
//        }
    }
}

void MyStrategy::getWarriorActions(const PlayerView& playerView, Actions& actions) {
    bool isAnyEnemy = false;
    for (const Entity& entity : playerView.entities) {
        if (entity.playerId != playerView.myId && entity.playerId != -1) {
            isAnyEnemy = true;
            break;
        }
    }
    if (!isAnyEnemy) {
        return;
    }

    const auto& units = playerView.GetMyEntities(RANGED_UNIT);
    const auto& meleeUnits = playerView.GetMyEntities(MELEE_UNIT);
    int rangedCount = units.size();
    for (const Entity& unit : units) {
        if (myToEnemyMapping[unit.id].empty()) {
            continue;
        }
        int minEnemyDist = myToEnemyMapping[unit.id].begin()->distance;
        int minEnemyId = myToEnemyMapping[unit.id].begin()->entityId;
//        std::cout << "id: " << unit.id << ", my position: (" << unit.position.x << ", " << unit.position.y
//                  << "), distance: " << minEnemyDist << std::endl;
        Vec2Int targetPosition = {19, 19};
        if (rangedCount > 30) {
            targetPosition = playerView.entitiesById.at(minEnemyId).position;
        }
        if (minEnemyDist < 20) {
            actions[unit.id] = AttackAction({20, {
                WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                MELEE_UNIT, RANGED_BASE, RANGED_UNIT, RESOURCE, TURRET
            }});
            continue;
        }
        actions[unit.id] = MoveAction(targetPosition, false, false);
    }
    for (const Entity& unit : meleeUnits) {
        actions[unit.id] = MoveAction({0, 0}, false, false);
    }
}

std::unordered_map<int, std::set<DistId>> MyStrategy::calculateDistances(
        const PlayerView& playerView,
        int keyPlayerId,
        int valuePlayerId
) {
    std::unordered_map<int, std::set<DistId>> result;
    for (const Entity& keyEntity : playerView.entities) {
        if (!eqPlayerTeam(keyEntity.playerId, keyPlayerId, playerView)) {
            continue;
        }
        result[keyEntity.id] = {};
        for (const Entity& valueEntity : playerView.entities) {
            if (eqPlayerTeam(valueEntity.playerId, valuePlayerId, playerView)) {
                result[keyEntity.id].insert({dist(keyEntity, valueEntity), valueEntity.id});
            }
        }
    }
    return result;
}

Vec2Int MyStrategy::findHousePlace() {
    int x = 2;
    int y = 2;
    while (true) {
        if (isEmptyForHouse(x, y)) {
            return {x, y};
        }
        if (x == 2) {
            x = y + 5;
            y = 2;
        } else {
            x -= 5;
            y += 5;
        }
    }
}

bool MyStrategy::isEmptyForHouse(int x, int y) {
    for (int i = x - 1; i < x + 1; ++i) {
        for (int j = y - 1; j < y + 1; ++j) {
            if (world[i][j] != -1) {
                return false;
            }
        }
    }
    return true;
}

BuildAction MyStrategy::createBuildUnitAction(const Entity& base, EntityType unitType, bool isAggresive) {
    if (isAggresive) {
        if (world[base.position.x + 4][base.position.y + 5] == -1) {
            return BuildAction(unitType, {base.position.x + 4, base.position.y + 5});
        }
        if (world[base.position.x + 5][base.position.y + 4] == -1) {
            return BuildAction(unitType, {base.position.x + 5, base.position.y + 4});
        }
    } else {
        if (world[base.position.x][base.position.y - 1] == -1) {
            return BuildAction(unitType, {base.position.x, base.position.y - 1});
        }
        if (world[base.position.x - 1][base.position.y] == -1) {
            return BuildAction(unitType, {base.position.x - 1, base.position.y});
        }
    }
    return BuildAction(unitType, {base.position.x, base.position.y - 3});
}
