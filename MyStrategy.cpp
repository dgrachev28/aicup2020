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

    if (buildersCount < 40) {
        for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
            actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, false);
        }
    } else {
        for (const Entity& rangedBase : playerView.GetMyEntities(RANGED_BASE)) {
            actions[rangedBase.id] = createBuildUnitAction(rangedBase, RANGED_UNIT, true);
        }
        if (buildersCount < 100) {
            for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, true);
            }
        } else {
            for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = EntityAction();
            }
        }
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
//    std::vector<Entity> potentialHouseBuilders;

    std::vector<std::pair<int, Vec2Int>> potentialHouseBuilders;
    EntityType buildType = HOUSE;

    if (myPlayer.resource >= 600) {
        std::vector<Vec2Int> basePositions;
        for (int i = 7; i < 60; i += 5) {
            for (int j = 7; j < 60; j += 5) {
                if (i + j <= 34) {
                    continue;
                }
                basePositions.emplace_back(i, j);
            }
        }
        for (const auto& position : basePositions) {
            int unitId = isEmptyForHouse(playerView, position.x, position.y, 2);
            if (unitId != -1) {
                potentialHouseBuilders.push_back({unitId, {position.x - 1, position.y - 1}});
                buildType = RANGED_BASE;
            }
        }
    }
    if (potentialHouseBuilders.empty()) {
        if (playerView.getFood() < 5 && myPlayer.resource >= 50) {
            std::vector<Vec2Int> housePositions;
            for (int i = 1; i < 60; i += 3) {
                housePositions.emplace_back(i, 1);
            }
            for (int i = 5; i < 60; i += 3) {
                housePositions.emplace_back(1, i);
            }
            for (int i = 7; i < 60; i += 5) {
                for (int j = 7; j < 60; j += 5) {
                    housePositions.emplace_back(i, j);
                }
            }

            for (const auto &housePosition : housePositions) {
                int unitId = isEmptyForHouse(playerView, housePosition.x, housePosition.y, 1);
                if (unitId != -1) {
                    potentialHouseBuilders.push_back({unitId, {housePosition.x - 1, housePosition.y - 1}});
                }
            }
        }
    }

    if (!potentialHouseBuilders.empty()) {
        std::sort(potentialHouseBuilders.begin(), potentialHouseBuilders.end(), [] (const std::pair<int, Vec2Int>& unit1, const std::pair<int, Vec2Int>& unit2) {
            return unit1.second.x + unit1.second.y < unit2.second.x + unit2.second.y;
        });
        const auto& unit = potentialHouseBuilders[0];
        houseBuilders.insert(unit.first);
        actions[unit.first] = BuildAction(buildType, {unit.second.x, unit.second.y});
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

    const auto& rangedUnits = playerView.GetMyEntities(RANGED_UNIT);
    const auto& meleeUnits = playerView.GetMyEntities(MELEE_UNIT);
    for (const Entity& unit : rangedUnits) {
        if (myToEnemyMapping[unit.id].empty()) {
            continue;
        }
        int minEnemyDist = myToEnemyMapping[unit.id].begin()->distance;
        int minEnemyId = myToEnemyMapping[unit.id].begin()->entityId;
//        std::cout << "id: " << unit.id << ", my position: (" << unit.position.x << ", " << unit.position.y
//                  << "), distance: " << minEnemyDist << std::endl;
        Vec2Int targetPosition = {19, 19};
        if (rangedUnits.size() + meleeUnits.size() > 20) {
            targetPosition = playerView.entitiesById.at(minEnemyId).position;
        }
        if (minEnemyDist < 20) {
            actions[unit.id] = AttackAction({1000, {
                WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
            }});
            continue;
        }
        actions[unit.id] = MoveAction(targetPosition, true, true);
    }
    for (const Entity& unit : meleeUnits) {
        if (myToEnemyMapping[unit.id].empty()) {
            continue;
        }
        int minEnemyDist = myToEnemyMapping[unit.id].begin()->distance;
        int minEnemyId = myToEnemyMapping[unit.id].begin()->entityId;
//        std::cout << "id: " << unit.id << ", my position: (" << unit.position.x << ", " << unit.position.y
//                  << "), distance: " << minEnemyDist << std::endl;
        Vec2Int targetPosition = {19, 19};
        if (rangedUnits.size() + meleeUnits.size() > 20) {
            targetPosition = playerView.entitiesById.at(minEnemyId).position;
        }
        if (minEnemyDist < 20) {
            actions[unit.id] = AttackAction({1000, {
                    WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                    MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
            }});
            continue;
        }
        actions[unit.id] = MoveAction(targetPosition, true, false);
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

bool MyStrategy::checkBuilderUnit(const PlayerView& playerView, int x, int y) {
    return world[x][y] != -1 && playerView.entitiesById.at(world[x][y]).entityType == BUILDER_UNIT
           && playerView.entitiesById.at(world[x][y]).playerId == playerView.myId;
}

int MyStrategy::isEmptyForHouse(const PlayerView& playerView, int x, int y, int size) {
    int unitSize = size + 1;
    for (int i = x - size; i <= x + size; ++i) {
        for (int j = y - size; j <= y + size; ++j) {
            if (world[i][j] != -1) {
                return -1;
            }
        }
    }
    if (size == 2) {
        std::vector<int> neighbours{world[x + 5][y + 5], world[x + 5][y - 5], world[x - 5][y + 5], world[x - 5][y - 5]};
        for (int n : neighbours) {
            if (n != -1 && playerView.entitiesById.at(n).entityType == RANGED_BASE) {
                return -1;
            }
        }
    }

    for (int i = x - size; i <= x + size; ++i) {
        if (y >= unitSize && checkBuilderUnit(playerView, i, y - unitSize)) {
            return world[i][y - unitSize];
        }
        if (y <= 79 - unitSize && checkBuilderUnit(playerView, i, y + unitSize)) {
            return world[i][y + unitSize];
        }
    }

    for (int i = y - size; i <= y + size; ++i) {
        if (x >= unitSize && checkBuilderUnit(playerView, x - unitSize, i)) {
            return world[x - unitSize][i];
        }
        if (x <= 79 - unitSize && checkBuilderUnit(playerView, x + unitSize, i)) {
            return world[x + unitSize][i];
        }
    }
    return -1;
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