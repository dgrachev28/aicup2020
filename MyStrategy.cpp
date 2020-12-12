#include "MyStrategy.hpp"
#include <exception>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <cmath>


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

std::vector<Vec2Int> getEdges(const Vec2Int& v, bool include = false) {
    std::vector<Vec2Int> edges;
    if (v.x + v.y % 2 == 0) {
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
    } else {
        if (v.y < 79) {
            edges.emplace_back(v.x, v.y + 1);
        }
        if (v.y > 0) {
            edges.emplace_back(v.x, v.y - 1);
        }
        if (v.x < 79) {
            edges.emplace_back(v.x + 1, v.y);
        }
        if (v.x > 0) {
            edges.emplace_back(v.x - 1, v.y);
        }
    }

    if (include) {
        edges.emplace_back(v.x, v.y);
    }
    return edges;
}

std::array<std::array<int, 80>, 80> MyStrategy::bfs(const std::vector<Vec2Int>& startCells) {
    std::array<std::array<int, 80>, 80> d;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            d[i][j] = 100000;
        }
    }
    std::queue<Vec2Int> q;
    for (const Vec2Int& cell : startCells) {
        d[cell.x][cell.y] = 0;
        q.push(cell);
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
        const std::vector<Vec2Int>& edges = edgesMap[v];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    return d;
}

std::array<std::array<int, 80>, 80> MyStrategy::dijkstra(const std::vector<Vec2Int>& startCells, bool isWeighted = true) {
    static std::unordered_set<EntityType> BUILDINGS = {HOUSE, MELEE_BASE, RANGED_BASE, TURRET};
//    std::vector<std::vector<int>> d{80, std::vector<int>(80, 1000000)};

    std::array<std::array<int, 80>, 80> d;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            d[i][j] = 100000;
        }
    }
    std::set<std::pair<int, Vec2Int>> q;
    for (const Vec2Int& cell : startCells) {
        d[cell.x][cell.y] = 0;
        q.insert(std::make_pair(d[cell.x][cell.y], cell));
    }
    while (!q.empty()) {
        Vec2Int v = q.begin()->second;
        q.erase(q.begin());

        for (const Vec2Int& edge : edgesMap[v]) {
            int len = 1;
            if (isWeighted) {
                len = 10;
                if (!world(edge).isEmpty()) {
                    const auto& type = world(edge).getEntityType();
                    if (BUILDINGS.count(type)) {
                        continue;
                    }
                    if (type == MELEE_UNIT || type == RANGED_UNIT || type == BUILDER_UNIT) {
                        len = 28;
                    }
                    if (type == RESOURCE) {
                        len = 28;
                    }
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

MyStrategy::MyStrategy() {
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            if (i > 0) {
                edgesMap[{i, j}].emplace_back(i - 1, j);
            }
            if (i < 79) {
                edgesMap[{i, j}].emplace_back(i + 1, j);
            }
            if (j > 0) {
                edgesMap[{i, j}].emplace_back(i, j - 1);
            }
            if (j < 79) {
                edgesMap[{i, j}].emplace_back(i, j + 1);
            }
        }
    }
    freeScoutSpots = {{9, 70}, {70, 9}, {70, 70}};
}

Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface) {
    this->playerView = &playerView;
    int anyEnemyId;
    for (const Player& player : playerView.players) {
        if (player.id != playerView.myId) {
            anyEnemyId = player.id;
        }
    }
//    myToMyMapping = calculateDistances(playerView, playerView.myId, playerView.myId);
    myToEnemyMapping = calculateDistances(playerView, playerView.myId, anyEnemyId);
    enemyToMyMapping = calculateDistances(playerView, anyEnemyId, playerView.myId);
    enemyToEnemyMapping = calculateDistances(playerView, anyEnemyId, anyEnemyId);

    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            world(i, j) = {i, j};
        }
    }
    for (auto& entity : playerView.entities) {
        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);
        for (int i = entity.position.x; i < entity.position.x + properties.size; ++i) {
            for (int j = entity.position.y; j < entity.position.y + properties.size; ++j) {
                world(i, j).entity = &entity;
            }
        }
        if (entity.entityType == TURRET && entity.playerId != playerView.myId && entity.active) {
            int myRangedCount = 0;
            for (const auto& distId : enemyToMyMapping[entity.id]) {
                if (distId.distance > 7) {
                    break;
                }
                if ((distId.distance == 6 || distId.distance == 7) && playerView.entitiesById.at(distId.entityId).entityType == RANGED_UNIT) {
                    ++myRangedCount;
                }
            }
            if (myRangedCount < 6) {
                for (int i = entity.position.x - 5; i <= entity.position.x + 6; ++i) {
                    for (int j = entity.position.y - 5; j <= entity.position.y + 6; ++j) {
                        if (!checkWorldBounds(i, j)) {
                            continue;
                        }
                        int distance = std::min({
                                                        dist({i, j}, entity.position),
                                                        dist({i, j}, {entity.position.x, entity.position.y + 1}),
                                                        dist({i, j}, {entity.position.x + 1, entity.position.y}),
                                                        dist({i, j}, {entity.position.x + 1, entity.position.y + 1})
                                                });
                        if (distance <= 5) {
                            world(i, j).turretDanger = true;
                        }
                    }
                }
            }
        }
    }

    std::unordered_map<int, EntityAction> actions;


    getFarmerActions(playerView, actions);
//    getWarriorActions(playerView, actions);
    getRangedUnitAction(playerView, actions);
    getBuildUnitActions(playerView, actions);

    handleMoves(actions);
    handleAttackActions(actions);
    this->playerView = nullptr;
    return actions;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface) {
    debugInterface.send(DebugCommand::Clear());
    debugInterface.getState();
}

void MyStrategy::getBuildUnitActions(const PlayerView& playerView, Actions& actions) {
    const auto& myPlayer = playerView.playersById.at(playerView.myId);

    int buildersCount = playerView.getMyEntities(BUILDER_UNIT).size();
    int rangedCount = playerView.getMyEntities(RANGED_UNIT).size();
    int meleesCount = playerView.getMyEntities(MELEE_UNIT).size();
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
        for (const Entity& builderBase : playerView.getMyEntities(BUILDER_BASE)) {
            actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, buildersCount > 10);
        }
    } else {
        int maxBuildersCount;
//        if (economicFactor > 5.0) {
//            maxBuildersCount = 35;
////        } else if (economicFactor > 3.0) {
////            maxBuildersCount = 60;
//        } else if (economicFactor > 2.0) {
//            maxBuildersCount = 35;
//        } else if (economicFactor > 0.5) {
//            maxBuildersCount = 20;
//        } else if (economicFactor > 0.1) {
//            maxBuildersCount = 5;
//        }
        int maxRangedCount;
//        if (playerView.playersById.at(playerView.myId).resource < 40) {
//            maxRangedCount = 20;
//            maxBuildersCount = 45;
//        }
        if (playerView.playersById.at(playerView.myId).resource < 500) {
            maxRangedCount = 40;
            maxBuildersCount = 45;
        } else if (playerView.playersById.at(playerView.myId).resource < 1000) {
            maxRangedCount = 45;
            maxBuildersCount = 50;
        } else {
            maxRangedCount = 100;
            maxBuildersCount = 50;
        }
        if (!topPotentials.empty()) {
            if (topPotentials[0].score.score > 100.0) {
                maxBuildersCount = 5;
            } else if (topPotentials[0].score.score > 80.0) {
                maxBuildersCount = 20;
            }
        }

        if (rangedCount < maxRangedCount) {
            for (const Entity& rangedBase : playerView.getMyEntities(RANGED_BASE)) {
                if (topPotentials.empty()) {
                    actions[rangedBase.id] = createBuildUnitAction(rangedBase, RANGED_UNIT, true);
                } else {
                    std::vector<Vec2Int> buildPositions;
                    buildPositions.reserve(20);
                    for (int i = 0; i < 5; ++i) {
                        buildPositions.emplace_back(rangedBase.position.x - 1, rangedBase.position.y + i);
                        buildPositions.emplace_back(rangedBase.position.x + i, rangedBase.position.y - 1);
                        buildPositions.emplace_back(rangedBase.position.x + 5, rangedBase.position.y + i);
                        buildPositions.emplace_back(rangedBase.position.x + i, rangedBase.position.y + 5);
                    }
                    std::sort(buildPositions.begin(), buildPositions.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return std::abs(topPotentials[0].x - v1.x) + std::abs(topPotentials[0].y - v1.y)
                                < std::abs(topPotentials[0].x - v2.x) + std::abs(topPotentials[0].y - v2.y);
                    });
                    for (const auto& pos : buildPositions) {
                        if (world(pos).isEmpty()) {
                            actions[rangedBase.id] = BuildAction(RANGED_UNIT, pos);
                            break;
                        }
                    }
                }
            }
        } else {
            for (const Entity& rangedBase : playerView.getMyEntities(RANGED_BASE)) {
                actions[rangedBase.id] = EntityAction();
            }
        }
//        if (meleesCount < 20) {
//            for (const Entity& base : playerView.getMyEntities(MELEE_BASE)) {
//                actions[base.id] = createBuildUnitAction(base, MELEE_UNIT, true);
//            }
//        } else {
//            for (const Entity& base : playerView.getMyEntities(MELEE_BASE)) {
//                actions[base.id] = EntityAction();
//            }
//        }

        if (buildersCount < maxBuildersCount) {
            for (const Entity& builderBase : playerView.getMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT, true);
            }
        } else {
            for (const Entity& builderBase : playerView.getMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = EntityAction();
            }
        }
    }
    for (const Entity& turret : playerView.getMyEntities(TURRET)) {
        actions[turret.id] = AttackAction({1000, {
                WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
        }});
    }

}

void MyStrategy::getFarmerActions(const PlayerView& playerView, Actions& actions) {
    std::unordered_set<EntityType> BUILDING_TYPES = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, TURRET, WALL};
    const auto& units = playerView.getMyEntities(BUILDER_UNIT);

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

    if (playerView.getMyEntities(RANGED_BASE).empty() && myPlayer.resource >= 470) {
        std::vector<Vec2Int> basePositions;
        for (int i = 5; i < 60; i += 1) {
            for (int j = 5; j < 60; j += 1) {
                if (i < 13 && j < 13) {
                    continue;
                }
                basePositions.emplace_back(i, j);
            }
        }
        for (const auto& position : basePositions) {
            int unitId = isEmptyForHouse(position.x, position.y, 2);
            if (unitId != -1) {
                potentialHouseBuilders.push_back({unitId, {position.x - 2, position.y - 2}});
                buildType = RANGED_BASE;
            }
        }
    }
    if (potentialHouseBuilders.empty()) {
        if (playerView.getFood() < 10 && myPlayer.resource >= 47 && playerView.getInactiveHousesCount() < 3
        && (!playerView.getMyEntities(RANGED_BASE).empty() || playerView.getMyEntities(HOUSE).size() < 4 || myPlayer.resource >= 600)) {
            std::vector<Vec2Int> housePositions;
            if (playerView.getMyEntities(HOUSE).empty()) {
                for (int i = 1; i < 60; ++i) {
                    if (i % 3 != 1) {
                        housePositions.emplace_back(i, 1);
                        housePositions.emplace_back(1, i);
                    }
                }
            } else {
//                housePositions.emplace_back(1, 1);
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

            for (const auto& housePosition : housePositions) {
                int unitId = isEmptyForHouse(housePosition.x, housePosition.y, 1);
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
        if (playerView.getMyEntities(HOUSE).empty()) {
            if (unit.second.x == 0 && unit.second.y == 0) {
                edgeHousesShiftX = 1;
                edgeHousesShiftY = 1;
            }
            if (unit.second.x == 0) {
                edgeHousesShiftX = 2;
                edgeHousesShiftY = unit.second.y % 3 + 1;
            }
            if (unit.second.y == 0) {
                edgeHousesShiftX = unit.second.x % 3 + 1;
                edgeHousesShiftY = 2;
            }
        }

        houseBuilders.insert(unit.first);
        actions[unit.first] = BuildAction(buildType, {unit.second.x, unit.second.y});
    }

    for (const Entity& unit : units) {
        bool isAfraidEnemy = false;
        for (const DistId& distId : myToEnemyMapping.at(unit.id)) {
            const auto& enemy = playerView.entitiesById.at(distId.entityId);
            if (enemy.entityType == RANGED_UNIT && distId.distance <= 7
                    || enemy.entityType == MELEE_UNIT && distId.distance <= 3) {
                for (const auto& edge : getEdges(unit.position)) {
                    if (world(edge).isEmpty() && dist(edge, enemy.position) > distId.distance) {
                        actions[unit.id] = MoveAction({edge.x, edge.y}, false, false);
                        isAfraidEnemy = true;
                        break;
                    }
                }
            }
            if (isAfraidEnemy || distId.distance > 7) {
                break;
            }
        }
        if (isAfraidEnemy) {
            continue;
        }
        if (houseBuilders.count(unit.id)) {
            continue;
        }

//        const BuilderMeta& meta = builderMeta.at(unit.id);
        const auto& edges = getEdges(unit.position);
        bool shouldRepair = false;
        for (const auto& edge : edges) {
            if (brokenBuildings.count(world(edge).getEntityId())) {
                actions[unit.id] = RepairAction(world(edge).getEntityId());
                shouldRepair = true;
                break;
            } else if (world(edge).isEmpty()) {
                const auto& edgesOfEdge = getEdges(edge);
                for (const auto& e : edgesOfEdge) {
                    if (brokenBuildings.count(world(e).getEntityId())) {
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
                         dist(keyEntity.position, {valueEntity.position.x, valueEntity.position.y + entitySize - 1}),
                         dist(keyEntity.position, {valueEntity.position.x + entitySize - 1, valueEntity.position.y}),
                         dist(keyEntity.position, {valueEntity.position.x + entitySize - 1, valueEntity.position.y + entitySize - 1})
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

bool MyStrategy::checkBuilderUnit(int x, int y) {
    return checkWorldBounds(x, y)
           && world(x, y).eqEntityType(BUILDER_UNIT)
           && world(x, y).eqPlayerId(playerView->myId);
}

int MyStrategy::isEmptyForHouse(int x, int y, int size) {
    int unitSize = size + 1;
    for (int i = x - size; i <= x + size; ++i) {
        for (int j = y - size; j <= y + size; ++j) {
            if (!world(i, j).isEmpty()) {
                return -1;
            }
        }
    }
//    if (size == 2) {
//        std::vector<int> neighbours{world[x + 5][y + 5], world[x + 5][y - 5], world[x - 5][y + 5], world[x - 5][y - 5]};
//        for (int n : neighbours) {
//            if (n != -1 && playerView.entitiesById.at(n).entityType == RANGED_BASE) {
//                return -1;
//            }
//        }
//    }

    for (int i = x - (size + 6); i <= x + (size + 6); ++i) {
        for (int j = y - (size + 6); j <= y + (size + 6); ++j) {
            if (checkWorldBounds(i, j) && world(i, j).eqEntityType(RANGED_UNIT)
                    && !world(i, j).eqPlayerId(playerView->myId)) {
                return -1;
            }
        }
    }

    for (int i = x - size; i <= x + size; ++i) {
        if (checkBuilderUnit(i, y - unitSize)) {
            return world(i, y - unitSize).getEntityId();
        }
        if (checkBuilderUnit(i, y + unitSize)) {
            return world(i, y + unitSize).getEntityId();
        }
    }

    for (int i = y - size; i <= y + size; ++i) {
        if (checkBuilderUnit(x - unitSize, i)) {
            return world(x - unitSize, i).getEntityId();
        }
        if (checkBuilderUnit(x + unitSize, i)) {
            return world(x + unitSize, i).getEntityId();
        }
    }
    return -1;
}

BuildAction MyStrategy::createBuildUnitAction(const Entity& base, EntityType unitType, bool isAggresive) {
    if (isAggresive) {
        if (world(base.position.x + 4, base.position.y + 5).isEmpty()) {
            return BuildAction(unitType, {base.position.x + 4, base.position.y + 5});
        }
        if (world(base.position.x + 5, base.position.y + 4).isEmpty()) {
            return BuildAction(unitType, {base.position.x + 5, base.position.y + 4});
        }
    } else {
        if (world(base.position.x, base.position.y - 1).isEmpty()) {
            return BuildAction(unitType, {base.position.x, base.position.y - 1});
        }
        if (world(base.position.x - 1, base.position.y).isEmpty()) {
            return BuildAction(unitType, {base.position.x - 1, base.position.y});
        }
    }
    return BuildAction(unitType, {base.position.x, base.position.y - 3});
}


void MyStrategy::getRangedUnitAction(const PlayerView& playerView, Actions& actions) {
    const auto& rangedUnits = playerView.getMyEntities(RANGED_UNIT);
    if (playerView.fogOfWar) {
        std::vector<int> scoutsToRemove;
        for (const auto& [unitId, target] : scouts) {
            if (!playerView.entitiesById.count(unitId)) {
                scoutsToRemove.push_back(unitId);
                freeScoutSpots.insert(target);
            }
        }
        for (int unitId : scoutsToRemove) {
            scouts.erase(unitId);
        }

        std::unordered_set<Vec2Int> newFreeScoutSpots;
        for (const auto& spot : freeScoutSpots) {
            int minDist = 100000;
            int scoutId = -1;
            for (const auto& unit : rangedUnits) {
                int distance = dist(unit.position, spot);
                if (!scouts.count(unit.id) && distance < minDist) {
                    minDist = distance;
                    scoutId = unit.id;
                }
            }
            if (scoutId != -1) {
                scouts[scoutId] = spot;
            } else {
                newFreeScoutSpots.insert(spot);
            }
        }
        freeScoutSpots = newFreeScoutSpots;
    } else {
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
    }

    findTargetEnemies(playerView);

    std::unordered_map<Vec2Int, std::array<std::array<int, 80>, 80>> dijkstraResults;
    std::unordered_map<int, Vec2Int> defenders;
    for (const auto& potential : topPotentials) {
        if (world(potential.x, potential.y).isEmpty()) {
            continue;
        }
        if (!dijkstraResults.count({potential.x, potential.y})) {
            dijkstraResults[{potential.x, potential.y}] = dijkstra({{potential.x, potential.y}});
        }
        int i = 0;
        for (const DistId& distId : enemyToMyMapping[world(potential.x, potential.y).entity->id]) {
            const Entity& myUnit = playerView.entitiesById.at(distId.entityId);
            if (myUnit.entityType == RANGED_UNIT && !defenders.count(myUnit.id)) {
                defenders[myUnit.id] = {potential.x, potential.y};
                ++i;
                if (i >= potential.count) {
                    break;
                }
            }
        }
    }

//    const auto& rangedUnits = playerView.getMyEntities(RANGED_UNIT);
    const auto& meleeUnits = playerView.getMyEntities(MELEE_UNIT);
    int armySize = rangedUnits.size() + meleeUnits.size();
    for (const Entity& unit : rangedUnits) {
        int minEnemyDist = 100000;
        int minEnemyId = -1;
        if (!myToEnemyMapping[unit.id].empty()) {
            minEnemyDist = myToEnemyMapping[unit.id].begin()->distance;
            minEnemyId = myToEnemyMapping[unit.id].begin()->entityId;
        }

//        std::cout << "id: " << unit.id << ", my position: (" << unit.position.x << ", " << unit.position.y
//                  << "), distance: " << minEnemyDist << std::endl;
        Vec2Int targetPosition = {19, 19};
        if ((playerView.currentTick > 100 || minEnemyDist < 30) && minEnemyId != -1) {
            if (defenders.count(unit.id)) {
                targetPosition = defenders[unit.id];
            } else {
                int minPotentialDist = 100000;
                for (const auto& potential : topPotentials) {
                    int distance = dist({potential.x, potential.y}, unit.position);
                    if (distance < minPotentialDist) {
                        minPotentialDist = distance;
                        targetPosition = {potential.x, potential.y};
                    }
                }
                for (const auto& potential : topAttackPotentials) {
                    int distance = dist({potential.x, potential.y}, unit.position);
                    if (distance < minPotentialDist) {
                        minPotentialDist = distance;
                        targetPosition = {potential.x, potential.y};
                    }
                }
            }
//            targetPosition = getWarriorTargetPosition(unit);
//            if (targetPosition == Vec2Int(0, 0)) {
//                targetPosition = playerView.entitiesById.at(minEnemyId).position;
//            }
        }
        if (playerView.fogOfWar && scouts.count(unit.id)) {
            targetPosition = scouts[unit.id];
        }
        if (minEnemyDist < 6) {
            actions[unit.id] = AttackAction({1000, {
                    WALL, HOUSE, BUILDER_BASE, BUILDER_UNIT, MELEE_BASE,
                    MELEE_UNIT, RANGED_BASE, RANGED_UNIT, TURRET
            }});
            continue;
        }
        if (!dijkstraResults.count(targetPosition)) {
            dijkstraResults[targetPosition] = dijkstra({targetPosition});
        }
        std::vector<Vec2Int> edges = getEdges(unit.position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return dijkstraResults.at(targetPosition)[v1.x][v1.y] < dijkstraResults.at(targetPosition)[v2.x][v2.y];
        });

        if (world(edges[0]).eqEntityType(RESOURCE)) {
            actions[unit.id] = AttackAction(world(edges[0]).getEntityId());
        } else {
            for (const auto& edge : edges) {
                addMove(unit.id, edge, dijkstraResults.at(targetPosition)[edge.x][edge.y], 10);
            }
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
        if (playerView.currentTick > 100 || minEnemyDist < 30) {
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
            dijkstraResults[targetPosition] = dijkstra({targetPosition});
        }
        std::vector<Vec2Int> edges = getEdges(unit.position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return dijkstraResults.at(targetPosition)[v1.x][v1.y] < dijkstraResults.at(targetPosition)[v2.x][v2.y];
        });

        if (world(edges[0]).eqEntityType(RESOURCE)) {
            actions[unit.id] = AttackAction(world(edges[0]).getEntityId());
        } else {
            for (const auto& edge : edges) {
                addMove(unit.id, edge, dijkstraResults.at(targetPosition)[edge.x][edge.y], 10);
            }
        }
    }
    moveBattleUnits(actions);
}

Vec2Int MyStrategy::getWarriorTargetPosition(const Entity &unit) {
    Vec2Int targetPosition;
    PotentialCell bestCell{0.0, 0, 0, 0};
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
            inertion = powDecay(dist(lastTargetPositions.at(unit.id), {topPotentials[i].x, topPotentials[i].y}), 0.5);
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
    static std::unordered_map<EntityType, float> unitScore = {{RANGED_UNIT, 1.0}, {MELEE_UNIT, 0.8}};
    const int kLargestDistance = 40;

    std::vector<PotentialCell> potentials;
    std::vector<PotentialCell> attackPotentials;
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
                    if (i < 7) {
                        avgBuilderDist += distance;
                        ++i;
                    }
                }
                if ((unit.entityType == HOUSE || unit.entityType == RANGED_BASE || unit.entityType == BUILDER_BASE || unit.entityType == MELEE_BASE) && buildingDist == 0) {
                    int entitySize = playerView.entityProperties.at(unit.entityType).size / 2;
                    buildingDist = dist(enemy.position, {unit.position.x + entitySize, unit.position.y + entitySize});
                }
                if ((unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) && distance < 15) {
                    myPowerScore += unitScore[unit.entityType] * linearDecay(std::max(distance - 3, 0), 30);
                }
            }

            for (const DistId& distId : enemyToEnemyMapping[enemy.id]) {
                const Entity& unit = playerView.entitiesById.at(distId.entityId);
                const auto& distance = dist(enemy, unit);
                if (distance >= 15) {
                    break;
                }
                if (unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) {
                    enemyPowerScore += unitScore[unit.entityType] * linearDecay(distance, 30);
                }
            }
            if (i > 0) {
                avgBuilderDist /= static_cast<float>(i);
            }
            float myBuildingScore = customDecay(buildingDist);
            float myBuilderScore = customDecay(avgBuilderDist);

            Score score{myBuildingScore, myBuilderScore, myPowerScore, enemyPowerScore};
            score.calcScore2();
            potentials.push_back({score, enemy.position.x, enemy.position.y, 1});
        }

        if (enemy.playerId != playerView.myId && (enemy.entityType == HOUSE || enemy.entityType == RANGED_BASE
                || enemy.entityType == MELEE_BASE || enemy.entityType == BUILDER_BASE || enemy.entityType == BUILDER_UNIT)) {
            float myPowerScore = 0.0;
//            float enemyPowerScore = 0.0;
            for (const DistId& distId : enemyToMyMapping[enemy.id]) {
                const Entity& unit = playerView.entitiesById.at(distId.entityId);
                const auto& distance = dist(enemy, unit);
                if ((unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) && distance < 30) {
                    myPowerScore += unitScore[unit.entityType] * linearDecay(distance, 50);
                }
            }
//
//            for (const DistId& distId : enemyToEnemyMapping[enemy.id]) {
//                const Entity& unit = playerView.entitiesById.at(distId.entityId);
//                const auto& distance = dist(enemy, unit);
//                if (distance >= 30) {
//                    break;
//                }
//                if (unit.entityType == RANGED_UNIT || unit.entityType == MELEE_UNIT) {
//                    enemyPowerScore += unitScore[unit.entityType] * linearDecay(distance, 50);
//                }
//            }

            Score score{0.0f, 0.0f, myPowerScore, 0.0f};
            score.calcAttackScore();
            attackPotentials.push_back({score, enemy.position.x, enemy.position.y});
        }

    }

    std::sort(potentials.begin(), potentials.end());
    std::reverse(potentials.begin(), potentials.end());
    topPotentials.clear();
    for (const auto& p : potentials) {
        bool isTooClose = false;
        for (auto& tp : topPotentials) {
            if (dist({tp.x, tp.y}, {p.x, p.y}) < 5) {
                isTooClose = true;
                ++tp.count;
                break;
            }
        }
        if (!isTooClose) {
            topPotentials.push_back(p);
        }
        if (topPotentials.size() >= 6) {
            break;
        }
    }

    std::sort(attackPotentials.begin(), attackPotentials.end());
    std::reverse(attackPotentials.begin(), attackPotentials.end());
    topAttackPotentials.clear();
    for (const auto& p : attackPotentials) {
        bool isTooClose = false;
        for (auto& tp : topAttackPotentials) {
            if (dist({tp.x, tp.y}, {p.x, p.y}) < 5) {
                isTooClose = true;
                ++tp.count;
                break;
            }
        }
        if (!isTooClose) {
            topAttackPotentials.push_back(p);
        }
        if (topAttackPotentials.size() >= 8) {
            break;
        }
    }
//    if (topPotentials.size() >= 6) {
//        std::cerr << "[0] tick: " << playerView.currentTick << ", score: " << topPotentials[0] << std::endl;
//        std::cerr << "[1] tick: " << playerView.currentTick << ", score: " << topPotentials[1] << std::endl;
//        std::cerr << "[2] tick: " << playerView.currentTick << ", score: " << topPotentials[2] << std::endl;
//        std::cerr << "[3] tick: " << playerView.currentTick << ", score: " << topPotentials[3] << std::endl;
//        std::cerr << "[4] tick: " << playerView.currentTick << ", score: " << topPotentials[4] << std::endl;
//        std::cerr << "[5] tick: " << playerView.currentTick << ", score: " << topPotentials[5] << std::endl;
//        std::cerr << "==========================================" << std::endl;
//    }
//    if (topPotentials.size() >= 6) {
//        std::cerr << "===============ATTACK=====================" << std::endl;
//        std::cerr << "[0] tick: " << playerView.currentTick << ", score: " << attackPotentials[0] << std::endl;
//        std::cerr << "[1] tick: " << playerView.currentTick << ", score: " << attackPotentials[1] << std::endl;
//        std::cerr << "[2] tick: " << playerView.currentTick << ", score: " << attackPotentials[2] << std::endl;
//        std::cerr << "[3] tick: " << playerView.currentTick << ", score: " << attackPotentials[3] << std::endl;
//        std::cerr << "[4] tick: " << playerView.currentTick << ", score: " << attackPotentials[4] << std::endl;
//        std::cerr << "[5] tick: " << playerView.currentTick << ", score: " << attackPotentials[5] << std::endl;
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

void MyStrategy::battleDfs(int unitId, std::unordered_set<int>& groupedUnits, std::unordered_map<int, int>& group) {
    const auto& mapping = playerView->entitiesById.at(unitId).playerId == playerView->myId
            ? myToEnemyMapping : enemyToMyMapping;
    if (groupedUnits.count(unitId)) {
        return;
    }
    groupedUnits.insert(unitId);
    for (const DistId& enemy : mapping.at(unitId)) {
        if (enemy.distance > 7) {
            break;
        }
        const Entity& enemyEntity = playerView->entitiesById.at(enemy.entityId);

        if ((enemyEntity.entityType == RANGED_UNIT && (enemy.distance <= 7 && enemy.distance >= 3)) ||
                (enemyEntity.entityType == MELEE_UNIT && (enemy.distance == 1 || enemy.distance == 2))) {
            if (!group.count(unitId)) {
                group[unitId] = enemy.distance;
            }
            battleDfs(enemy.entityId, groupedUnits, group);
        }
    }
}

void MyStrategy::moveBattleUnits(Actions& actions) {
    std::vector<Entity> battleUnits;
    std::unordered_set<int> groupedUnits;
    std::vector<std::unordered_map<int, int>> groups;

    std::vector<Vec2Int> enemiesPositions;
    std::vector<Vec2Int> myPositions;
    for (const Entity& unit : playerView->entities) {
        if (unit.playerId != playerView->myId && unit.entityType == RANGED_UNIT) {
            enemiesPositions.push_back(unit.position);
        }
    }

    for (const Entity& unit : playerView->getMyEntities(RANGED_UNIT)) {
        myPositions.push_back(unit.position);
    }
    const auto& enemyMap = bfs(enemiesPositions);
    const auto& myMap = bfs(myPositions);

    for (const Entity& unit : playerView->getMyEntities(RANGED_UNIT)) {
        if (groupedUnits.count(unit.id)) {
            continue;
        }
        std::unordered_map<int, int> group;
//        std::cerr << "current tick: " << playerView->currentTick << ", NEW GROUP: " << unit.position.x << ", " << unit.position.y << std::endl;
        battleDfs(unit.id, groupedUnits, group);
        groups.push_back(group);
    }
    for (const auto& group : groups) {
        if (group.empty()) {
            continue;
        }
        int my0 = 0, enemy0 = 0, my1 = 0, enemy1 = 0, my2 = 0, enemy2 = 0;
//        std::unordered_map<int, int> enemies;
//        std::unordered_map<int, int> mine;
        float avgPosition = 0.0;
        for (const auto& [unitId, d] : group) {
            const Entity& unit = playerView->entitiesById.at(unitId);
            if (unit.playerId == playerView->myId) {
                int distance = enemyMap[unit.position.x][unit.position.y];
                if (distance == 1 || distance == 6) {
                    ++my1;
                } else if (distance == 2 || distance == 7) {
                    ++my2;
                } else if (distance >= 3 && distance <= 5) {
                    ++my0;
                }
            } else {
                int distance = myMap[unit.position.x][unit.position.y];
                if (distance == 1 || distance == 6) {
                    ++enemy1;
                } else if (distance == 2 || distance == 7) {
                    ++enemy2;
                } else if (distance >= 3 && distance <= 5) {
                    ++enemy0;
                }
            }
            avgPosition += playerView->entitiesById.at(unitId).position.x + playerView->entitiesById.at(unitId).position.y;
        }
        if (!group.empty()) {
            avgPosition /= group.size();
        }
//        std::cerr << "current tick: " << playerView->currentTick << ", group size: " << group.size()
//                  << ", my0: " << my0 << ", enemy0: " << enemy0
//                  << ", my1: " << my1 << ", enemy1: " << enemy1
//                  << ", my2: " << my2 << ", enemy2: " << enemy2
//                  << std::endl;
        bool afraid;
        if (avgPosition < 50) {
            afraid = (my0 + my1 + my2 < enemy0 + enemy1 + enemy2 && enemy0 + enemy1 + enemy2 > 0)
                     || (my0 + my1 < enemy0 + enemy1 && enemy0 + enemy1 > 0);
        } else {
            afraid = (my0 + my1 + my2 <= enemy0 + enemy1 + enemy2 && enemy0 + enemy1 + enemy2 > 0)
                     || (my0 + my1 <= enemy0 + enemy1 && enemy0 + enemy1 > 0);
            if (my0 + my1 == enemy0 + enemy1 && (my0 + my1 + my2 > enemy0 + enemy1 + enemy2)) {
                afraid = false;
            }
        }

        if (afraid) {
            for (const auto& [unitId, distance] : group) {
                const Entity& unit = playerView->entitiesById.at(unitId);
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] > 5) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, true);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    std::reverse(edges.begin(), edges.end());
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 1);
                    }
                }
            }
        }
    }
}

void MyStrategy::addMove(int unitId, const Vec2Int &target, int score, int priority) {
    if (!movePriorityToUnitIds[priority].count(unitId)) {
        unitMoveSteps[unitId].clear();
    }
    movePriorityToUnitIds[priority].insert(unitId);
    unitMoveSteps[unitId].push_back({unitId, target, score});
}

void MyStrategy::handleMoves(Actions& actions) {
    static std::unordered_set<EntityType> UNITS = {MELEE_UNIT, RANGED_UNIT, BUILDER_UNIT};
    std::vector<std::vector<int>> moveWorld{80, std::vector<int>(80, -1)};
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            if (world(i, j).entity && !UNITS.count(world(i, j).getEntityType())) {
                moveWorld[i][j] = -2;
            }
        }
    }

    std::unordered_set<int> movedUnitIds;
    for (const auto& [priority, unitIds] : movePriorityToUnitIds) {
        for (int unitId : unitIds) {
            if (movedUnitIds.count(unitId)) {
                continue;
            }
            const std::vector<MoveStep>& moves = unitMoveSteps.at(unitId);
            if (moves.empty()) {
                throw std::runtime_error("Moves shouldn't be empty");
            }
            for (const MoveStep& moveStep : moves) {
                int cellValue = moveWorld[moveStep.target.x][moveStep.target.y];
                if (cellValue != -1) {
                    continue;
                }
                int currentPosMovedUnitId = moveWorld[playerView->entitiesById.at(unitId).position.x][playerView->entitiesById.at(unitId).position.y];
                if (currentPosMovedUnitId >= 0 && playerView->entitiesById.at(currentPosMovedUnitId).position == moveStep.target) {
                    continue;
                }
                if (world(moveStep.target).turretDanger) {
                    continue;
                }
                moveWorld[moveStep.target.x][moveStep.target.y] = unitId;
                actions[unitId] = MoveAction(moveStep.target, false, false);
                break;
            }
            movedUnitIds.insert(unitId);
        }
    }
    unitMoveSteps.clear();
    movePriorityToUnitIds.clear();

}

Cell& MyStrategy::world(int x, int y) {
    return world_[x][y];
}

Cell& MyStrategy::world(const Vec2Int& v) {
    return world(v.x, v.y);
}

bool MyStrategy::checkWorldBounds(int x, int y) {
    return x >= 0 && x < 80 && y >= 0 && y < 80;
}

void MyStrategy::handleAttackActions(Actions& actions) {
    std::unordered_map<int, int> enemyHealth;
    std::unordered_map<int, std::unordered_set<int>> myToEnemy;
    std::unordered_map<int, std::unordered_set<int>> enemyToMy;

    for (const auto& myEntity : playerView->getMyEntities(RANGED_UNIT)) {
        if (actions.count(myEntity.id) && actions[myEntity.id].moveAction && actions[myEntity.id].moveAction->target != myEntity.position) {
            continue;
        }
        for (const auto& distId : myToEnemyMapping[myEntity.id]) {
            if (distId.distance > 5) {
                break;
            }
            const auto& enemy = playerView->entitiesById.at(distId.entityId);
            const auto& type = playerView->entitiesById.at(distId.entityId).entityType;
            enemyHealth[enemy.id] = enemy.health;
            myToEnemy[myEntity.id].insert(enemy.id);
            enemyToMy[enemy.id].insert(myEntity.id);
        }
    }

    int i = 0;
    while (!myToEnemy.empty() && !enemyToMy.empty() && i < 20) {
        resolveSimpleShoots(enemyHealth, myToEnemy, enemyToMy, actions);
        std::unordered_map<int, int> enemyToMyCounts;
        std::vector<int> enemies;
        for (const auto& [enemyId, myIds] : enemyToMy) {
            enemyToMyCounts[enemyId] = myIds.size();
            if (!myIds.empty()) {
                enemies.push_back(enemyId);
            }
        }

        if (!enemies.empty()) {
            std::sort(enemies.begin(), enemies.end(), [&](int enemy1, int enemy2) {
                EntityType type1 = playerView->entitiesById.at(enemy1).entityType;
                EntityType type2 = playerView->entitiesById.at(enemy2).entityType;
                if ((type1 == RANGED_UNIT || type1 == MELEE_UNIT) && !(type2 == RANGED_UNIT || type2 == MELEE_UNIT)) {
                    return true;
                }
                if (!(type1 == RANGED_UNIT || type1 == MELEE_UNIT) && (type2 == RANGED_UNIT || type2 == MELEE_UNIT)) {
                    return false;
                }
                if ((enemyHealth[enemy1] + 4) / 5 == enemyToMyCounts[enemy1] &&
                    (enemyHealth[enemy2] + 4) / 5 != enemyToMyCounts[enemy2]) {
                    return true;
                }
                if ((enemyHealth[enemy1] + 4) / 5 != enemyToMyCounts[enemy1] &&
                    (enemyHealth[enemy2] + 4) / 5 == enemyToMyCounts[enemy2]) {
                    return false;
                }
                if ((enemyHealth[enemy1] + 4) / 5 < enemyToMyCounts[enemy1] &&
                    (enemyHealth[enemy2] + 4) / 5 > enemyToMyCounts[enemy2]) {
                    return true;
                }
                if ((enemyHealth[enemy1] + 4) / 5 > enemyToMyCounts[enemy1] &&
                    (enemyHealth[enemy2] + 4) / 5 < enemyToMyCounts[enemy2]) {
                    return false;
                }
                return type1 == BUILDER_UNIT;
            });
            if (enemyToMy[enemies[0]].empty()) {
                throw std::runtime_error("Mapping shouldn't be empty");
            }
            shoot(*enemyToMy[enemies[0]].begin(), enemies[0], enemyHealth, myToEnemy, enemyToMy, actions);
        }
        ++i;
    }
}

void MyStrategy::resolveSimpleShoots(
        std::unordered_map<int, int>& enemyHealth,
        std::unordered_map<int, std::unordered_set<int>>& myToEnemy,
        std::unordered_map<int, std::unordered_set<int>>& enemyToMy,
        Actions& actions
) {
    bool resolved = false;
    int i = 0;
    while (!resolved && i <= 10) {
        std::unordered_map<int, int> pairsToShoot;
        for (const auto& [myId, enemies] : myToEnemy) {
            if (enemies.size() == 1) {
                pairsToShoot[myId] = *enemies.begin();
            }
        }
        resolved = pairsToShoot.empty();
        for (const auto& [myId, enemyId] : pairsToShoot) {
            shoot(myId, enemyId, enemyHealth, myToEnemy, enemyToMy, actions);
        }
        ++i;
    }
}

void MyStrategy::shoot(
        int myId,
        int enemyId,
        std::unordered_map<int, int>& enemyHealth,
        std::unordered_map<int, std::unordered_set<int>>& myToEnemy,
        std::unordered_map<int, std::unordered_set<int>>& enemyToMy,
        Actions& actions
) {
    enemyHealth[enemyId] -= 5;
    bool killed = enemyHealth[enemyId] == 0;
    if (killed) {
        enemyHealth.erase(enemyId);
    }

    for (int enemyIdKey : myToEnemy[myId]) {
        enemyToMy[enemyIdKey].erase(myId);
    }
    if (killed) {
        for (int myIdKey : enemyToMy[enemyId]) {
            myToEnemy[myIdKey].erase(enemyId);
        }
        enemyToMy.erase(enemyId);
    }
    myToEnemy.erase(myId);

    actions[myId] = AttackAction(enemyId);
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
    powerScore = (enemyPowerScore - myPowerScore) * 2.0f;
    score = myBuilderScore + myBuildingScore + powerScore;
}

void Score::calcScore2() {
    score = myBuilderScore + myBuildingScore;
}

void Score::calcAttackScore() {
//    powerScore = std::max((1 + myPowerScore - enemyPowerScore) * 2.0f, 0.0f);
    score = myPowerScore;
}
