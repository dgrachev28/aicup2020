#include "MyStrategy.hpp"
#include <exception>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <cmath>


//std::vector<int> bfs(int s) {
//    // длина любого кратчайшего пути не превосходит n - 1,
//    // поэтому n - достаточное значение для "бесконечности";
//    // после работы алгоритма dist[v] = n, если v недостижима из s
//    std::vector<int> dist(1000, 1000);
//    dist[s] = 0;
//    std::queue<int> q;
//    q.push(s);
//
//    while (!q.empty()) {
//        int v = q.front();
//        q.pop();
//        for (int u : adj[v]) {
//            if (dist[u] > dist[v] + 1) {
//                dist[u] = dist[v] + 1;
//                q.push(u);
//            }
//        }
//    }
//
//    return dist;
//}

float customDecay(float value) {
    float a = 50;
    float b = 5;
    float maxValue = 150;
    if (value > maxValue) {
        return 0.0;
    }
    if (value < a - b) {
        return a - value;
    }
    float x1 = b - a;
    return (value - maxValue) * b / (x1 - maxValue);
}

float powDecay(int distance, float a = 0.2f) {
    if (distance == 0) {
        return 1.0f;
    }
    return 1.0f / std::pow(distance, a);
}

float linearDecay(int distance, int maxDistance) {
    if (distance > maxDistance) {
        return 0.0f;
    }
    return 1.0f - static_cast<float>(distance) / static_cast<float>(maxDistance);
}

std::vector<Vec2Int> getEdges(const Vec2Int& v) {
    std::vector<Vec2Int> edges;
    if (v.x > 0) {
        edges.emplace_back(v.x - 1, v.y);
    }
    if (v.x < 79) {
        edges.emplace_back(v.x + 1, v.y);
    }
    if (v.y > 0) {
        edges.emplace_back(v.x, v.y - 1);
    }
    if (v.y < 79) {
        edges.emplace_back(v.x, v.y + 1);
    }
    return edges;
}


std::vector<std::vector<int>> MyStrategy::dijkstra(int x, int y) {
    static std::unordered_set<EntityType> BUILDINGS = {HOUSE, MELEE_BASE, RANGED_BASE, TURRET};
    std::vector<std::vector<int>> d{80, std::vector<int>(80, 1000000)};
    d[x][y] = 0;
    std::set<std::pair<int, Vec2Int>> q;
    q.insert(std::make_pair(d[x][y], Vec2Int{x, y}));
    while (!q.empty()) {
        Vec2Int v = q.begin()->second;
        q.erase(q.begin());

        std::vector<Vec2Int> edges = getEdges(v);
        for (const Vec2Int& edge : edges) {
            int len = 10;
            if (world[edge.x][edge.y] != -1) {
                const auto& type = playerView->entitiesById.at(world[edge.x][edge.y]).entityType;
                if (BUILDINGS.count(type)) {
                    continue;
                }
                if (type == MELEE_UNIT || type == RANGED_UNIT || type == BUILDER_UNIT) {
                    len = 28;
                }
                if (type == RESOURCE) {
                    len = 30;
                }
            }

            if (d[v.x][v.y] + len < d[edge.x][edge.y]) {
                q.erase(std::make_pair(d[edge.x][edge.y], edge));
                d[edge.x][edge.y] = d[v.x][v.y] + len;
                q.insert(std::make_pair(d[edge.x][edge.y], edge));
            }
        }
    }
    return d;
}

int dist(const Entity& e1, const Entity& e2) {
    return std::abs(e1.position.x - e2.position.x) + std::abs(e1.position.y - e2.position.y);
}

int dist(const Vec2Int& p1, const Vec2Int& p2) {
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
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
    this->playerView = &playerView;
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
    enemyToMyMapping = calculateDistances(playerView, anyEnemyId, playerView.myId);
    enemyToEnemyMapping = calculateDistances(playerView, anyEnemyId, anyEnemyId);

    std::unordered_map<int, EntityAction> actions;

    getBuildUnitActions(playerView, actions);
    getFarmerActions(playerView, actions);
//    getWarriorActions(playerView, actions);
    getRangedUnitAction(playerView, actions);

    this->playerView = nullptr;
    return actions;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface) {
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}

void MyStrategy::getBuildUnitActions(const PlayerView& playerView, Actions& actions) {
    const auto& myPlayer = playerView.playersById.at(playerView.myId);

    int buildersCount = playerView.GetMyEntities(BUILDER_UNIT).size();
    int rangedCount = playerView.GetMyEntities(RANGED_UNIT).size();
    int meleesCount = playerView.GetMyEntities(MELEE_UNIT).size();
    int enemiesCount = 0;
    int resourcesCount = 0;
    for (const auto& entity : playerView.entities) {
        if ((entity.entityType == MELEE_UNIT || entity.entityType == RANGED_UNIT) && entity.playerId != playerView.myId) {
            enemiesCount += 1;
        } else if (entity.entityType == RESOURCE) {
            resourcesCount += 1;
        }
    }
    float economicFactor = static_cast<float>(resourcesCount) / (static_cast<float>(enemiesCount) + 0.0001f);
//    std::cerr << "currentTick: " << playerView.currentTick << "economicFactor: " << economicFactor << std::endl;
    if (playerView.currentTick < 100 && buildersCount < 20) {
        for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
            actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, buildersCount > 10);
        }
    } else {
        int maxBuildersCount;
        if (economicFactor > 5.0) {
            maxBuildersCount = 50;
//        } else if (economicFactor > 3.0) {
//            maxBuildersCount = 60;
        } else if (economicFactor > 2.0) {
            maxBuildersCount = 40;
        } else if (economicFactor > 0.5) {
            maxBuildersCount = 20;
        } else if (economicFactor > 0.1) {
            maxBuildersCount = 5;
        }
        int maxRangedCount;
        if (playerView.playersById.at(playerView.myId).resource < 500) {
            maxRangedCount = 30;
        } else if (playerView.playersById.at(playerView.myId).resource < 2000) {
            maxRangedCount = 40;
        } else if (playerView.playersById.at(playerView.myId).resource < 5000) {
            maxRangedCount = 50;
        } else {
            maxRangedCount = 60;
        }

        if (rangedCount < maxRangedCount) {
            for (const Entity& rangedBase : playerView.GetMyEntities(RANGED_BASE)) {
                actions[rangedBase.id] = createBuildUnitAction(rangedBase, RANGED_UNIT, true);
            }
        } else {
            for (const Entity& rangedBase : playerView.GetMyEntities(RANGED_BASE)) {
                actions[rangedBase.id] = EntityAction();
            }
        }
//        if (meleesCount < 20) {
//            for (const Entity& base : playerView.GetMyEntities(MELEE_BASE)) {
//                actions[base.id] = createBuildUnitAction(base, MELEE_UNIT, true);
//            }
//        } else {
//            for (const Entity& base : playerView.GetMyEntities(MELEE_BASE)) {
//                actions[base.id] = EntityAction();
//            }
//        }

        if (buildersCount < maxBuildersCount) {
            for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, true);
            }
        } else {
            for (const Entity& builderBase : playerView.GetMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = EntityAction();
            }
        }
    }
    for (const Entity& turret : playerView.GetMyEntities(TURRET)) {
        actions[turret.id] = AttackAction({1000, {
                WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
        }});
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

//    if ((myPlayer.resource >= 600 && playerView.GetMyEntities(RANGED_BASE).size() < 2)
//            || (myPlayer.resource >= 2000 && playerView.GetMyEntities(RANGED_BASE).size() < 3)
//            || myPlayer.resource >= 6000 && playerView.GetMyEntities(RANGED_BASE).size() < 4) {
//        std::vector<Vec2Int> basePositions;
//        for (int i = 5; i < 60; i += 1) {
//            for (int j = 5; j < 60; j += 1) {
//                if (i + j <= 34) {
//                    continue;
//                }
//                basePositions.emplace_back(i, j);
//            }
//        }
//        for (const auto& position : basePositions) {
//            int unitId = isEmptyForHouse(playerView, position.x, position.y, 2);
//            if (unitId != -1) {
//                potentialHouseBuilders.push_back({unitId, {position.x - 2, position.y - 2}});
//                buildType = RANGED_BASE;
//            }
//        }
//    }
    if (potentialHouseBuilders.empty()) {
        if (playerView.getFood() < 10 && myPlayer.resource >= 50 && playerView.getInactiveHousesCount() < 3) {
            std::vector<Vec2Int> housePositions;
            if (playerView.GetMyEntities(HOUSE).empty()) {
                for (int i = 1; i < 60; ++i) {
                    housePositions.emplace_back(i, 1);
                    housePositions.emplace_back(1, i);
                }
            } else {
                housePositions.emplace_back(1, 1);
                for (int i = edgeHousesShiftX + 3; i < 60; i += 3) {
                    housePositions.emplace_back(i, 1);
                }
                for (int i = edgeHousesShiftY + 3; i < 60; i += 3) {
                    housePositions.emplace_back(1, i);
                }
            }
            for (int i = 7; i < 60; i += 5) {
                housePositions.emplace_back(i, 6);
                housePositions.emplace_back(6, i);
            }
            for (int i = 12; i < 60; i += 5) {
                for (int j = 12; j < 60; j += 5) {
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
        if (playerView.GetMyEntities(HOUSE).empty()) {
            if (unit.second.x == 0 && unit.second.y == 0) {
                edgeHousesShiftX = 1;
                edgeHousesShiftY = 1;
            }
            if (unit.second.x == 0) {
                edgeHousesShiftX = unit.second.y % 3 == 0 ? 2 : 1;
                edgeHousesShiftY = unit.second.y % 3 + 1;
            }
            if (unit.second.y == 0) {
                edgeHousesShiftX = unit.second.x % 3 + 1;
                edgeHousesShiftY = unit.second.x % 3 == 0 ? 2 : 1;
            }
        }

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
        const auto& edges = getEdges(unit.position);
        bool shouldRepair = false;
        for (const auto& edge : edges) {
            if (brokenBuildings.count(world[edge.x][edge.y])) {
                actions[unit.id] = RepairAction(world[edge.x][edge.y]);
                shouldRepair = true;
                break;
            } else if (world[edge.x][edge.y] == -1) {
                const auto& edgesOfEdge = getEdges(edge);
                for (const auto& e : edgesOfEdge) {
                    if (brokenBuildings.count(world[e.x][e.y])) {
                        actions[unit.id] = MoveAction({edge.x, edge.y}, false, false);
                        shouldRepair = true;
                        break;
                    }
                }
            }
        }
        if (shouldRepair) {
            continue;
        }
        actions[unit.id] = AttackAction({1000, {RESOURCE}});
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
            if (eqPlayerTeam(valueEntity.playerId, valuePlayerId, playerView) && keyEntity.id != valueEntity.id) {
                int entitySize = playerView.entityProperties.at(valueEntity.entityType).size;
                int distance;
                if (entitySize > 1) {
                    distance = std::min({
                         dist(keyEntity.position, valueEntity.position),
                         dist(keyEntity.position, {valueEntity.position.x, valueEntity.position.y + entitySize}),
                         dist(keyEntity.position, {valueEntity.position.x + entitySize, valueEntity.position.y}),
                         dist(keyEntity.position, {valueEntity.position.x + entitySize, valueEntity.position.y + entitySize})
                    });
                } else {
                    distance = dist(keyEntity, valueEntity);
                };
                result[keyEntity.id].insert({distance, valueEntity.id});
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

    for (int i = x - (size + 6); i <= x + (size + 6); ++i) {
        for (int j = y - (size + 6); j <= y + (size + 6); ++j) {
            if (i >= 0 && j >= 0 && i <= 79 && j <= 79 && world[i][j] != -1
                    && playerView.entitiesById.at(world[i][j]).entityType == RANGED_UNIT
                    && playerView.entitiesById.at(world[i][j]).playerId != playerView.myId) {
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



void MyStrategy::getRangedUnitAction(const PlayerView& playerView, Actions& actions) {
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

    findTargetEnemies(playerView);

    std::unordered_map<Vec2Int, std::vector<std::vector<int>>> dijkstraResults;

    const auto& rangedUnits = playerView.GetMyEntities(RANGED_UNIT);
    const auto& meleeUnits = playerView.GetMyEntities(MELEE_UNIT);
    int armySize = rangedUnits.size() + meleeUnits.size();
    for (const Entity& unit : rangedUnits) {
        if (myToEnemyMapping[unit.id].empty()) {
            continue;
        }
        int minEnemyDist = myToEnemyMapping[unit.id].begin()->distance;
        int minEnemyId = myToEnemyMapping[unit.id].begin()->entityId;
//        std::cout << "id: " << unit.id << ", my position: (" << unit.position.x << ", " << unit.position.y
//                  << "), distance: " << minEnemyDist << std::endl;
        Vec2Int targetPosition = {19, 19};
        if (playerView.currentTick > 100 || minEnemyDist < 20) {
            targetPosition = getWarriorTargetPosition(unit);
            if (targetPosition == Vec2Int(0, 0)) {
                targetPosition = playerView.entitiesById.at(minEnemyId).position;
            }
        }
        if (minEnemyDist < 6) {
            actions[unit.id] = AttackAction({1000, {
                    WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                    MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
            }});
            continue;
        }
        if (!dijkstraResults.count(targetPosition)) {
            dijkstraResults[targetPosition] = dijkstra(targetPosition.x, targetPosition.y);
        }
        std::vector<Vec2Int> edges = getEdges(unit.position);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return dijkstraResults.at(targetPosition)[v1.x][v1.y] < dijkstraResults.at(targetPosition)[v2.x][v2.y];
        });

        if (world[edges[0].x][edges[0].y] != -1 && playerView.entitiesById.at(world[edges[0].x][edges[0].y]).entityType == RESOURCE) {
            actions[unit.id] = AttackAction(playerView.entitiesById.at(world[edges[0].x][edges[0].y]).id);
        } else {
            actions[unit.id] = MoveAction(edges[0], true, true);
        }
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
        if (playerView.currentTick > 100 || minEnemyDist < 20) {
            targetPosition = getWarriorTargetPosition(unit);
            if (targetPosition == Vec2Int(0, 0)) {
                targetPosition = playerView.entitiesById.at(minEnemyId).position;
            }
        }
        if (minEnemyDist < 3) {
            actions[unit.id] = AttackAction({1000, {
                    WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                    MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
            }});
            continue;
        }
        if (!dijkstraResults.count(targetPosition)) {
            dijkstraResults[targetPosition] = dijkstra(targetPosition.x, targetPosition.y);
        }
        std::vector<Vec2Int> edges = getEdges(unit.position);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return dijkstraResults.at(targetPosition)[v1.x][v1.y] < dijkstraResults.at(targetPosition)[v2.x][v2.y];
        });

        if (world[edges[0].x][edges[0].y] != -1 && playerView.entitiesById.at(world[edges[0].x][edges[0].y]).entityType == RESOURCE) {
            actions[unit.id] = AttackAction(playerView.entitiesById.at(world[edges[0].x][edges[0].y]).id);
        } else {
            actions[unit.id] = MoveAction(edges[0], true, true);
        }
    }
}

Vec2Int MyStrategy::getWarriorTargetPosition(const Entity &unit) {
    Vec2Int targetPosition;
    PotentialCell bestCell{0.0, 0, 0};
    float bestScore = -1000.0;
    float bestPotentialScore = 1000.0;
    for (int i = 0; i < 50; ++i) {
        if (topPotentials.size() == i) {
            break;
        }
        int distance = dist(unit.position, {topPotentials[i].x, topPotentials[i].y});
        float distScore = powDecay(distance, 0.4);
        float inertion = 1.0f;
        if (lastTargetPositions.count(unit.id)) {
            inertion = powDecay(dist(lastTargetPositions.at(unit.id), {topPotentials[i].x, topPotentials[i].y}), 0.2);
        }

        float cellScore = topPotentials[i].score.score * distScore * inertion;
        if (cellScore > bestScore) {
            bestCell = topPotentials[i];
            bestScore = cellScore;
            bestPotentialScore = topPotentials[i].score.score;
        }
    }
//    if (bestPotentialScore < 30.0 || playerView->currentTick > 500) {
        targetPosition = {bestCell.x, bestCell.y};
//    } else {
//        targetPosition = {19, 19};
//    }
    lastTargetPositions[unit.id] = targetPosition;
    return targetPosition;
}


void MyStrategy::findTargetEnemies(const PlayerView& playerView) {
    static std::unordered_map<EntityType, float> unitScore = {{RANGED_UNIT, 1.0}, {MELEE_UNIT, 0.6}};
    const int kLargestDistance = 40;

    std::vector<PotentialCell> potentials;
    for (const auto& enemy : playerView.entities) {
        if (enemy.playerId != playerView.myId && (enemy.entityType == RANGED_UNIT || enemy.entityType == MELEE_UNIT)) {
            int i = 0;
            float avgBuilderDist = 0.0;
            int buildingDist = 0;
            float myPowerScore = 0.0;
            float enemyPowerScore = 0.0;
            for (const DistId& distId : enemyToMyMapping[enemy.id]) {
                const Entity& unit = playerView.entitiesById.at(distId.entityId);
                const auto& distance = dist(enemy, unit);
                if (unit.entityType == BUILDER_UNIT) {
                    if (i < 5) {
                        avgBuilderDist += distance;
                        ++i;
                    }
                }
                if ((unit.entityType == HOUSE || unit.entityType == RANGED_BASE || unit.entityType == MELEE_BASE) && buildingDist == 0) {
                    int entitySize = playerView.entityProperties.at(unit.entityType).size / 2;
                    buildingDist = dist(enemy.position, {unit.position.x + entitySize, unit.position.y + entitySize});
                }
                if ((unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) && distance < 10) {
                    myPowerScore += unitScore[unit.entityType] * linearDecay(distance, 15);
                }
            }

            for (const DistId& distId : enemyToEnemyMapping[enemy.id]) {
                const Entity& unit = playerView.entitiesById.at(distId.entityId);
                const auto& distance = dist(enemy, unit);
                if (distance >= 10) {
                    break;
                }
                if (unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) {
                    enemyPowerScore += unitScore[unit.entityType] * linearDecay(distance, 15);
                }
            }
            if (i > 0) {
                avgBuilderDist /= static_cast<float>(i);
            }
            float myBuildingScore = customDecay(buildingDist);
            float myBuilderScore = customDecay(avgBuilderDist);

            Score score{myBuildingScore, myBuilderScore, myPowerScore, enemyPowerScore};
            score.calcScore();
            potentials.push_back({score, enemy.position.x, enemy.position.y});
        }
    }

    std::sort(potentials.begin(), potentials.end());
    std::reverse(potentials.begin(), potentials.end());
    topPotentials.clear();
    for (const auto& p : potentials) {
        bool isTooClose = false;
        for (const auto& tp : topPotentials) {
            if (dist({tp.x, tp.y}, {p.x, p.y}) < 5) {
                isTooClose = true;
                break;
            }
        }
        if (!isTooClose) {
            topPotentials.push_back(p);
        }
        if (topPotentials.size() >= 10) {
            break;
        }
    }
//    if (topPotentials.size() >= 3) {
//        std::cerr << "[0] tick: " << playerView.currentTick << ", score: " << topPotentials[0] << std::endl;
//        std::cerr << "[1] tick: " << playerView.currentTick << ", score: " << topPotentials[1] << std::endl;
//        std::cerr << "[2] tick: " << playerView.currentTick << ", score: " << topPotentials[2] << std::endl;
//        std::cerr << "==========================================" << std::endl;
//    }
}

void MyStrategy::createPotentialField(const PlayerView& playerView) {
//    for (int i = 0; i < 80; ++i) {
//        for (int j = 0; j < 80; ++j) {
//            potentialField[i][j] = 0;
//        }
//    }
//
//    static std::unordered_map<EntityType, float> unitScore = {{RANGED_UNIT, 1.0}, {MELEE_UNIT, 0.5}};
//    const int kLargestDistance = 30;
//    // TODO: change loop header
//    for (const auto& myEntity : playerView.entitiesByPlayerId.at(playerView.myId).at(EntityType::BUILDER_UNIT)) {
//        float myScore = 0.0;
//        float enemyScore = 0.0;
//        for (const DistId& distId : entitiesMapping[myEntity.id]) {
//            if (distId.distance > kLargestDistance) {
//                break;
//            }
//            float distScore;
//            const Entity& otherEntity = playerView.entitiesById.at(distId.entityId);
//            if (otherEntity.playerId == playerView.myId) {
//                if (distId.distance <= 5) {
//                    distScore = 1.0 - (distId.distance / 10.0);
//                } else {
//                    distScore = 0.0;
//                }
//                myScore += unitScore[otherEntity.entityType] * distScore;
//            } else {
//                distScore = 1.0 - (distId.distance / kLargestDistance);
//                enemyScore += unitScore[otherEntity.entityType] * distScore;
//            }
//        }
//
//        float winScore = std::min(myScore - enemyScore, 1.0f);
////        float potential = 1.0 - winScore;
////        if (potential < 0) {
////            potential = 0.0;
////        }
////        potential = potential * 10.0 + (kLargestDistance - loseDistance);
//        fillPotential(myEntity.position.x, myEntity.position.y, winScore);
//    }
//    topPotentials.clear();
//    for (int i = 0; i < 80; ++i) {
//        for (int j = 0; j < 80; ++j) {
//            topPotentials.push_back(PotentialCell{potentialField[i][j], i, j});
//        }
//    }
//    std::sort(topPotentials.begin(), topPotentials.end());
//    std::reverse(topPotentials.begin(), topPotentials.end());
//
//    std::cerr << "tick: " << playerView.currentTick << "score: " << topPotentials[0].score << ", x: "
//              << topPotentials[0].x << ", y: " << topPotentials[0].y << std::endl;
}


void MyStrategy::fillPotential(int x, int y, float score) {
    const int kFieldSize = 3;
    for (int i = -kFieldSize; i <= kFieldSize; ++i) {
        for (int j = -kFieldSize; j <= kFieldSize; ++j) {
            int distance = std::abs(i) + std::abs(j);
            if (distance <= kFieldSize) {
                potentialField[x + i][y + j] += score * (1.0f - static_cast<float>(distance) / kFieldSize);
//                potentialField[x + i][y + j] += score / (static_cast<float>(distance) + 1);
            }
        }
    }
}

std::ostream& operator<<(std::ostream& out, const Score& score) {
    out << "{ TOTAL SCORE: " << score.score
        << ", myBuildingScore: " << score.myBuildingScore << ", myBuilderScore: " << score.myBuilderScore
        << ", myPowerScore: " << score.myPowerScore << ", enemyPowerScore: " << score.enemyPowerScore
        << ", powerScore: " << score.powerScore << " }";
    return out;
}

std::ostream& operator<<(std::ostream& out, const PotentialCell& cell) {
    out << "{ score: " << cell.score << ", x: " << cell.x << ", y: " << cell.y << " }";
    return out;
}

void Score::calcScore() {
    powerScore = std::max((1 + enemyPowerScore - myPowerScore) * 3.0f, 0.0f);
    score = myBuilderScore + myBuildingScore + powerScore;
}
