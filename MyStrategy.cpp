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

std::array<std::array<int, 80>, 80>
MyStrategy::bfs(const std::vector<Vec2Int>& startCells, const std::unordered_set<EntityType>& obstacleTypes,
                const std::vector<int>& obstacleUnitIds) {
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
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1
                    && !world(edge).inEntityTypes(obstacleTypes)
                    && std::find(obstacleUnitIds.begin(), obstacleUnitIds.end(), world(edge).getEntityId()) == obstacleUnitIds.end()) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    return d;
}

std::array<std::array<int, 80>, 80>
MyStrategy::bfs(const std::vector<Vec2Int>& startCells,
                const std::unordered_set<EntityType>& obstacleTypes,
                const std::vector<int>& obstacleUnitIds,
                std::vector<int>& closestUnits) {
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
        if (closestUnits.size() < 10 && world(v).eqEntityType(BUILDER_UNIT) && world(v).eqPlayerId(playerView->myId)) {
            closestUnits.push_back(world(v).getEntityId());
        }
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1
                    && !world(edge).inEntityTypes(obstacleTypes)
                    && std::find(obstacleUnitIds.begin(), obstacleUnitIds.end(), world(edge).getEntityId()) == obstacleUnitIds.end()) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    return d;
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
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    return d;
}

std::vector<Vec2Int> MyStrategy::bfsBuilderResources(const std::vector<Vec2Int>& startCells,
                                                     const std::unordered_set<EntityType>& obstacleTypes = {},
                                                     const std::unordered_set<int>& obstacleUnitIds = {}) {
    std::array<std::array<bool, 80>, 80> visited;
    std::array<std::array<bool, 80>, 80> pushed;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            visited[i][j] = false;
            pushed[i][j] = false;
        }
    }
    std::vector<Vec2Int> builderResourcesCells;
    std::queue<Vec2Int> q;
    for (const Vec2Int& cell : startCells) {
        q.push(cell);
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (!world(edge).inEntityTypes(obstacleTypes) && !visited[edge.x][edge.y]) {
                visited[edge.x][edge.y] = true;
                q.push(edge);
            }
            if (world(edge).eqEntityType(RESOURCE)
                    && !pushed[v.x][v.y]
                    && !obstacleUnitIds.contains(world(v).getEntityId())) {
                pushed[v.x][v.y] = true;
                builderResourcesCells.push_back(v);
            }
        }
    }
    return builderResourcesCells;
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

        for (const Vec2Int& edge : edgesMap[v.x][v.y]) {
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
                edgesMap[i][j].emplace_back(i - 1, j);
            }
            if (i < 79) {
                edgesMap[i][j].emplace_back(i + 1, j);
            }
            if (j > 0) {
                edgesMap[i][j].emplace_back(i, j - 1);
            }
            if (j < 79) {
                edgesMap[i][j].emplace_back(i, j + 1);
            }
        }
    }
}

Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface) {
    this->playerView = &playerView;
    stepInit(playerView);

    std::unordered_map<int, EntityAction> actions;

    setBuilderUnitsActions(actions);
    getRangedUnitAction(playerView, actions);
    getBuildUnitActions(playerView, actions);

    handleMoves(actions);
    handleAttackActions(actions);
    handleBuilderAttackActions(actions);

    for (const auto& [unitId, action] : actions) {
//        if (playerView.entitiesById.at(unitId).position.x > 30
//                && playerView.entitiesById.at(unitId).entityType == RANGED_UNIT) {
//            if (action.moveAction) {
//                std::cerr << "unitId: " << unitId << ", move target: " << action.moveAction->target << std::endl;
//            }
//            if (action.attackAction) {
//                std::cerr << "unitId: " << unitId << ", attack target: " << *action.attackAction->target << std::endl;
//            }
//        }
        if (unitId < 0) {
            std::cerr << "ERRROR: unitId: " << unitId << std::endl;
        }
        if (action.moveAction) {
            if (action.moveAction->target.x < 0 || action.moveAction->target.x >= 80
                || action.moveAction->target.y < 0 || action.moveAction->target.y >= 80) {
                std::cerr << "ERRROR: move target: " << action.moveAction->target << std::endl;
            }
        }
        if (action.attackAction && *action.attackAction->target < 0) {
            std::cerr << "ERRROR: attack target: " << *action.attackAction->target << std::endl;
        }
        if (action.repairAction && action.repairAction->target < 0) {
            std::cerr << "ERRROR: repair target: " << action.repairAction->target << std::endl;
        }
        if (action.buildAction) {
            if (action.buildAction->position.x < 0 || action.buildAction->position.x >= 80
                || action.buildAction->position.y < 0 || action.buildAction->position.y >= 80) {
                std::cerr << "ERRROR: build target: " << action.buildAction->position << std::endl;
            }
        }
    }

    this->playerView = nullptr;
    return actions;
}

void MyStrategy::stepInit(const PlayerView& playerView) {
    isFinal = playerView.players.size() == 2;

    int anyEnemyId;
    for (const Player& player : playerView.players) {
        if (player.id != playerView.myId) {
            anyEnemyId = player.id;
        }
    }
    myToMyMapping = calculateDistances(playerView, playerView.myId, playerView.myId);
    myToEnemyMapping = calculateDistances(playerView, playerView.myId, anyEnemyId);
    enemyToMyMapping = calculateDistances(playerView, anyEnemyId, playerView.myId);
    enemyToEnemyMapping = calculateDistances(playerView, anyEnemyId, anyEnemyId);

    std::vector<Vec2Int> enemiesPositions;
    std::vector<Vec2Int> myPositions;
    for (const Entity& unit : playerView.entities) {
        if (unit.playerId != playerView.myId && unit.entityType == RANGED_UNIT) {
            enemiesPositions.push_back(unit.position);
        }
    }

    for (const Entity& unit : playerView.getMyEntities(RANGED_UNIT)) {
        myPositions.push_back(unit.position);
    }
    enemyMap = bfs(enemiesPositions);
    myMap = bfs(myPositions);

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
    if (playerView.currentTick < 100 && buildersCount < 20) {
        for (const Entity& builderBase : playerView.getMyEntities(BUILDER_BASE)) {
            actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT);
        }
    } else {
        int maxBuildersCount;
        int maxRangedCount;
        if (isFinal) {
            if (playerView.playersById.at(playerView.myId).resource < 500) {
                maxRangedCount = 40;
                maxBuildersCount = 70;
            } else if (playerView.playersById.at(playerView.myId).resource < 1000) {
                maxRangedCount = 45;
                maxBuildersCount = 80;
            } else {
                maxRangedCount = 80;
                maxBuildersCount = 80;
            }
        } else {
            if (playerView.playersById.at(playerView.myId).resource < 500) {
                maxRangedCount = 40;
                maxBuildersCount = 50;
            } else if (playerView.playersById.at(playerView.myId).resource < 1000) {
                maxRangedCount = 45;
                maxBuildersCount = 60;
            } else {
                maxRangedCount = 150;
                maxBuildersCount = 60;
            }
        }

        if (!topPotentials.empty()) {
            if (topPotentials[0].score.score > 85.0) {
                maxBuildersCount = 5;
            } else if (topPotentials[0].score.score > 80.0) {
                maxBuildersCount = 20;
            }
        }

        if (rangedCount < maxRangedCount) {
            for (const Entity& rangedBase : playerView.getMyEntities(RANGED_BASE)) {
                Vec2Int target{70, 70};
                if (!topPotentials.empty()) {
                    target = {topPotentials[0].x, topPotentials[0].y};
                }
                actions[rangedBase.id] = createBuildUnitAction2(rangedBase, RANGED_UNIT, target);
            }
        } else {
            for (const Entity& rangedBase : playerView.getMyEntities(RANGED_BASE)) {
                actions[rangedBase.id] = EntityAction();
            }
        }
        if (buildersCount < maxBuildersCount) {
            for (const Entity& builderBase : playerView.getMyEntities(BUILDER_BASE)) {
                actions[builderBase.id] = createBuildUnitAction(builderBase, BUILDER_UNIT);
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

std::unordered_map<int, std::vector<DistId>> MyStrategy::calculateDistances(
        const PlayerView& playerView,
        int keyPlayerId,
        int valuePlayerId
) {
    std::unordered_map<int, std::vector<DistId>> result;
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
                result[keyEntity.id].push_back({distance, valueEntity.id});
            }
        }
        std::sort(result[keyEntity.id].begin(), result[keyEntity.id].end());
    }
    return result;
}

bool MyStrategy::checkBuilderUnit(int x, int y, const std::unordered_set<int>& busyBuilders) {
    return checkWorldBounds(x, y)
           && world(x, y).eqEntityType(BUILDER_UNIT)
           && world(x, y).eqPlayerId(playerView->myId)
           && !busyBuilders.contains(world(x, y).getEntityId());
}

int MyStrategy::calcBlockingFarmScore(int x, int y, int size) {
    int aroundSize = size + 2;
    for (int i = x - aroundSize; i <= x + aroundSize; ++i) {
        for (int j = y - aroundSize; j <= y + aroundSize; ++j) {
            if (!world(i, j).isEmpty()) {
                return -1;
            }
        }
    }
    return -1;
}

PotentialBuilder MyStrategy::calcBuildingPlaceScore(int x, int y, int size, const std::unordered_set<int>& busyBuilders) {
    static std::unordered_set<EntityType> obstacleTypes = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, WALL, TURRET, RESOURCE};
    const auto& startCells = getBuildingEdges({x - size, y - size}, size * 2 + 1, true);

    std::array<std::array<int, 80>, 80> d;
        for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            d[i][j] = 100000;
        }
    }

    std::vector<DistPositionId> closestUnits;
    std::queue<Vec2Int> q;
    for (const Vec2Int& cell : startCells) {
        d[cell.x][cell.y] = 0;
        q.push(cell);
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
        if (world(v).eqEntityType(BUILDER_UNIT) && world(v).eqPlayerId(playerView->myId)) {
            closestUnits.push_back({d[v.x][v.y], v, world(v).getEntityId()});
        }
        int maxEmptyDistance = size == 2 ? 7 : 5;
        if (closestUnits.size() >= 4 || (closestUnits.empty() && d[v.x][v.y] > maxEmptyDistance) || d[v.x][v.y] > 10) {
            break;
        }
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1
                    && !world(edge).inEntityTypes(obstacleTypes)
                    && !busyBuilders.contains(world(edge).getEntityId())) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    if (closestUnits.empty()) {
        return {-1, 100000.0f, {0, 0}};
    }
    float score = 0.0f;
    for (const auto& distId : closestUnits) {
        score += static_cast<float>(distId.distance);
    }
    score /= closestUnits.size();
    return {closestUnits[0].entityId, score, {x, y}};
}

bool MyStrategy::isEmptyForHouse(int x, int y, int size, const std::unordered_set<int>& busyBuilders) {
    static std::unordered_set<EntityType> BUILDINGS = {HOUSE, BUILDER_BASE, MELEE_BASE, RANGED_BASE, TURRET};
    int unitSize = size + 1;
    for (int i = x - size; i <= x + size; ++i) {
        for (int j = y - size; j <= y + size; ++j) {
            if (!world(i, j).isEmpty()) {
                return false;
            }
        }
    }
    for (int i = x - unitSize; i <= x + unitSize; ++i) {
        for (int j = y - unitSize; j <= y + unitSize; ++j) {
            if (checkWorldBounds(i, j) && world(i, j).inEntityTypes(BUILDINGS)) {
                return false;
            }
        }
    }
    for (int i = x - (size + 6); i <= x + (size + 6); ++i) {
        for (int j = y - (size + 6); j <= y + (size + 6); ++j) {
            if (checkWorldBounds(i, j) && world(i, j).eqEntityType(RANGED_UNIT)
                    && !world(i, j).eqPlayerId(playerView->myId)) {
                return false;
            }
        }
    }
    return true;
}

EntityAction MyStrategy::createBuildUnitAction(const Entity& base, EntityType unitType) {
    if (unitType == BUILDER_UNIT && !farmTargets_.empty()) {
        std::sort(farmTargets_.begin(), farmTargets_.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            int distance1 = std::min({
                    dist(v1, base.position),
                    dist(v1, {base.position.x, base.position.y + 1}),
                    dist(v1, {base.position.x, base.position.y + 2}),
                    dist(v1, {base.position.x, base.position.y + 3}),
                    dist(v1, {base.position.x, base.position.y + 4}),
                    dist(v1, {base.position.x + 1, base.position.y + 4}),
                    dist(v1, {base.position.x + 1, base.position.y}),
                    dist(v1, {base.position.x + 2, base.position.y + 4}),
                    dist(v1, {base.position.x + 2, base.position.y}),
                    dist(v1, {base.position.x + 3, base.position.y + 4}),
                    dist(v1, {base.position.x + 3, base.position.y}),
                    dist(v1, {base.position.x + 4, base.position.y}),
                    dist(v1, {base.position.x + 4, base.position.y + 1}),
                    dist(v1, {base.position.x + 4, base.position.y + 2}),
                    dist(v1, {base.position.x + 4, base.position.y + 3}),
                    dist(v1, {base.position.x + 4, base.position.y + 4})
            });
            int distance2 = std::min({
                    dist(v2, base.position),
                    dist(v2, {base.position.x, base.position.y + 1}),
                    dist(v2, {base.position.x, base.position.y + 2}),
                    dist(v2, {base.position.x, base.position.y + 3}),
                    dist(v2, {base.position.x, base.position.y + 4}),
                    dist(v2, {base.position.x + 1, base.position.y + 4}),
                    dist(v2, {base.position.x + 1, base.position.y}),
                    dist(v2, {base.position.x + 2, base.position.y + 4}),
                    dist(v2, {base.position.x + 2, base.position.y}),
                    dist(v2, {base.position.x + 3, base.position.y + 4}),
                    dist(v2, {base.position.x + 3, base.position.y}),
                    dist(v2, {base.position.x + 4, base.position.y}),
                    dist(v2, {base.position.x + 4, base.position.y + 1}),
                    dist(v2, {base.position.x + 4, base.position.y + 2}),
                    dist(v2, {base.position.x + 4, base.position.y + 3}),
                    dist(v2, {base.position.x + 4, base.position.y + 4})
            });
            return distance1 < distance2;
        });
        return createBuildUnitAction2(base, unitType, farmTargets_[0]);
    }
    return EntityAction();
}

EntityAction MyStrategy::createBuildUnitAction2(const Entity& base, EntityType unitType, const Vec2Int& target) {
//    std::cerr << "tick: " << playerView->currentTick << ", TARGET: " << target << std::endl;
    std::vector<Vec2Int> buildPositions;
    buildPositions.reserve(20);
    for (int i = 0; i < 5; ++i) {
        std::vector<Vec2Int> positions{
                {base.position.x - 1, base.position.y + i},
                {base.position.x + i, base.position.y - 1},
                {base.position.x + 5, base.position.y + i},
                {base.position.x + i, base.position.y + 5}
        };
        for (const auto& p : positions) {
            if (checkWorldBounds(p.x, p.y)) {
                buildPositions.push_back(p);
            }
        }
    }
    std::sort(buildPositions.begin(), buildPositions.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
        return std::abs(target.x - v1.x) + std::abs(target.y - v1.y)
               < std::abs(target.x - v2.x) + std::abs(target.y - v2.y);
    });
    for (const auto& pos : buildPositions) {
        if (world(pos).isEmpty()) {
            return BuildAction(unitType, pos);
        }
    }
    return EntityAction();
}


void MyStrategy::getRangedUnitAction(const PlayerView& playerView, Actions& actions) {
    const auto& rangedUnits = playerView.getMyEntities(RANGED_UNIT);
    if (playerView.fogOfWar) {
        if (scouts.empty() && freeScoutSpots.empty()) {
//            if (isFinal) {
//                freeScoutSpots = {{70, 70}};
//            }
            freeScoutSpots = {{9, 70}, {70, 9}, {70, 70}};
        }
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
        if (playerView.fogOfWar) {
            if (isFinal) {
                targetPosition = {70, 70};
            } else {
                targetPosition = {9, 70};
            }
        }
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
            builderAttackActions[unit.id] = AttackAction(world(edges[0]).getEntityId());
        }
        for (const auto& edge : edges) {
            addMove(unit.id, edge, dijkstraResults.at(targetPosition)[edge.x][edge.y], 10);
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
//    std::cerr << "================== DEFENSE ===================" << std::endl;
//    for (const auto& tp : topPotentials) {
//        std::cerr << "tick: " << playerView.currentTick << ", score: " << tp << std::endl;
//    }
//    std::cerr << "================== ATTACK ====================" << std::endl;
//    for (const auto& tp : attackPotentials) {
//        std::cerr << "tick: " << playerView.currentTick << ", score: " << tp << std::endl;
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
        int minDefenseDist = 10000;
        int groupMyUnitId = -1;
        for (const auto& [unitId, distance] : group) {
            if (playerView->entitiesById.at(unitId).playerId == playerView->myId) {
                groupMyUnitId = unitId;
            }
        }
        if (groupMyUnitId != -1) {
            for (const DistId& distId : myToMyMapping.at(groupMyUnitId)) {
                EntityType type = playerView->entitiesById.at(distId.entityId).entityType;
                if (type == BUILDER_UNIT || type == HOUSE || type == BUILDER_BASE || type == RANGED_BASE) {
                    minDefenseDist = distId.distance;
                    break;
                }
            }
        }
//        std::cerr << "current tick: " << playerView->currentTick << ", group size: " << group.size()
//                  << ", my0: " << my0 << ", enemy0: " << enemy0
//                  << ", my1: " << my1 << ", enemy1: " << enemy1
//                  << ", my2: " << my2 << ", enemy2: " << enemy2
//                  << std::endl;
        MicroState microState;
        if (minDefenseDist < 10) {
            if (my1 >= enemy1 + enemy2) {
                microState = MicroState::ATTACK;
            } else if (my1 * 2 + my2 >= enemy1 * 2 + enemy2) {
                microState = MicroState::STAY;
            } else {
                microState = MicroState::RUN_AWAY;
            }
        } else {
            if (my1 > enemy1 + enemy2) {
                microState = MicroState::ATTACK;
            } else if (my1 * 2 + my2 > enemy1 * 2 + enemy2) {
                microState = MicroState::STAY;
            } else {
                microState = MicroState::RUN_AWAY;
            }
        }


        if (microState == MicroState::RUN_AWAY) {
            for (const auto& [unitId, distance] : group) {
                const Entity& unit = playerView->entitiesById.at(unitId);
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 6) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, true);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    std::reverse(edges.begin(), edges.end());
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 1);
                    }
                }
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 7) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, false);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    std::reverse(edges.begin(), edges.end());

                    addMove(unit.id, unit.position, 10, 1);
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 2);
                    }
                }
            }
        } else if (microState == MicroState::STAY) {
            for (const auto& [unitId, distance] : group) {
                const Entity& unit = playerView->entitiesById.at(unitId);
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 6) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, false);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    std::reverse(edges.begin(), edges.end());

                    addMove(unit.id, unit.position, 10, 1);
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 1);
                    }
                }
            }
        }
    }
}

void MyStrategy::addMove(int unitId, const Vec2Int& target, int score, int priority) {
    if (!movePriorityToUnitIds[priority].count(unitId)) {
        unitMoveSteps[unitId].clear();
    }
    movePriorityToUnitIds[priority].insert(unitId);
    unitMoveSteps[unitId].push_back({unitId, target, score});
}

void MyStrategy::handleMoves(Actions& actions) {
    std::unordered_set<int> debugUnits;
    static std::unordered_set<EntityType> UNITS = {MELEE_UNIT, RANGED_UNIT, BUILDER_UNIT};
    std::array<std::array<int, 80>, 80> moveWorld;
    std::array<std::array<int, 80>, 80> buildersWorld;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            if (world(i, j).entity && !UNITS.count(world(i, j).getEntityType())) {
                moveWorld[i][j] = -2;
            } else {
                moveWorld[i][j] = -1;
            }
            buildersWorld[i][j] = -1;
        }
    }
    for (int builderUnitId : movePriorityToUnitIds[25]) {
        const std::vector<MoveStep>& moves = unitMoveSteps.at(builderUnitId);
        if (moves.empty()) {
            throw std::runtime_error("Moves shouldn't be empty");
        }
        buildersWorld[moves[0].target.x][moves[0].target.y] = builderUnitId;
    }

    std::unordered_set<int> movedUnitIds;
    std::unordered_set<Vec2Int> stuckedUnits;
    for (const auto& [priority, unitIds] : movePriorityToUnitIds) {
        std::queue<Vec2Int> potentialStuckedQueue;
        for (int unitId : movePriorityToUnitIds[25]) {
            potentialStuckedQueue.push(playerView->entitiesById.at(unitId).position);
        }
        while (!potentialStuckedQueue.empty()) {
            const auto& unitPos = potentialStuckedQueue.front();
            potentialStuckedQueue.pop();
            if (moveWorld[unitPos.x][unitPos.y] != -1) {
                continue;
            }
            const auto& edges = edgesMap[unitPos.x][unitPos.y];
            Vec2Int nextPotentialStuckedUnitPos;
            int freeEdgesCount = 0;
            for (const auto& edge : edges) {
                if (moveWorld[edge.x][edge.y] == -1) {
                    ++freeEdgesCount;
                    nextPotentialStuckedUnitPos = edge;
                }
            }
            if (freeEdgesCount == 1
                    && !stuckedUnits.contains(unitPos)
                    && movePriorityToUnitIds[25].contains(world(unitPos).getEntityId())) {
                moveWorld[unitPos.x][unitPos.y] = world(unitPos).getEntityId();
                actions[world(unitPos).getEntityId()] = MoveAction(unitPos, false, false);
                movedUnitIds.insert(world(unitPos).getEntityId());
//                stuckedUnits.insert(unitPos);
                potentialStuckedQueue.push(nextPotentialStuckedUnitPos);
            }
        }

        std::queue<int> unitsQueue;
        for (int unitId : unitIds) {
            int currentPosMovedUnitId = moveWorld[playerView->entitiesById.at(unitId).position.x][playerView->entitiesById.at(unitId).position.y];
            if (currentPosMovedUnitId >= 0) {
                unitsQueue.push(unitId);
            }
        }
        while (!unitsQueue.empty()) {
            int unitId = unitsQueue.front();
            if (priority == 10) {
                std::cerr << "Tick: " << playerView->currentTick << ", unitId: " << unitId << std::endl;
            }
            unitsQueue.pop();

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
                if (stuckedUnits.contains(moveStep.target) && moveStep.target != playerView->entitiesById.at(unitId).position) {
                    continue;
                }
                moveWorld[moveStep.target.x][moveStep.target.y] = unitId;
                actions[unitId] = MoveAction(moveStep.target, false, false);
                if (priority == 10) {
                    std::cerr << "prior 10 QUEUE move: " << moveStep.target << ", unit_pos: " << playerView->entitiesById.at(unitId).position << ", unit_id: " << unitId << std::endl;
                }
                if (unitIds.contains(world(moveStep.target.x, moveStep.target.y).getEntityId())) {
                    unitsQueue.push(world(moveStep.target.x, moveStep.target.y).getEntityId());
                }
                break;
            }
            movedUnitIds.insert(unitId);
        }
        if (priority != 25) {
            std::unordered_map<Vec2Int, CollisionPriority> collisionsMap;
            std::unordered_map<EntityPtr, std::vector<Vec2Int>> potentialMoves;
            for (int unitId : unitIds) {
                if (movedUnitIds.count(unitId)) {
                    continue;
                }
                const Entity& unit = playerView->entitiesById.at(unitId);
                const std::vector<MoveStep>& moves = unitMoveSteps.at(unitId);
                if (moves.empty()) {
                    throw std::runtime_error("Moves shouldn't be empty");
                }

                std::optional<int> previousScore;
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
                    if (stuckedUnits.contains(moveStep.target) && moveStep.target != playerView->entitiesById.at(unitId).position) {
                        continue;
                    }
                    if (!previousScore || (*previousScore == moveStep.score && buildersWorld[moveStep.target.x][moveStep.target.y] == -1)) {
                        if (collisionsMap.contains(moveStep.target)) {
                            collisionsMap[moveStep.target].units.push_back(&unit);
                        } else {
                            collisionsMap[moveStep.target] = {moveStep.score, {&unit}, moveStep.target};
                        }
                        potentialMoves[&unit].push_back(moveStep.target);
                        previousScore = moveStep.score;
                    }
                }
            }

            std::set<CollisionPriority> collisions;
            for (const auto& [pos, collision] : collisionsMap) {
                collisions.insert(collision);
                if (priority < 2) {
                    std::cerr << "Tick: " << playerView->currentTick << ", collision: " << collision << std::endl;
                }
            }
            while (!collisions.empty()) {
                CollisionPriority collision = CollisionPriority(*collisions.begin());
                if (collision.units.empty()) {
                    throw std::runtime_error("Collisions are empty");
                }
                const Entity& unit = *collision.units[collision.units.size() - 1];
                bool moved = false;
                for (const auto& potentialMove : potentialMoves[&unit]) {
                    auto iterator = collisions.find(collisionsMap[potentialMove]);
                    if (iterator == collisions.end()) {
                        continue;
                    }
                    moved = true;
                    CollisionPriority collisionToChange = CollisionPriority(*iterator);
                    collisions.erase(iterator);
                    if (collisionToChange.units.size() > 1) {
                        collisionToChange.units.erase(std::find(collisionToChange.units.begin(), collisionToChange.units.end(), &unit));
                        collisions.insert(collisionToChange);
                        collisionsMap[potentialMove] = collisionToChange;
                    }
                }
                if (moved) {
                    moveWorld[collision.position.x][collision.position.y] = unit.id;
                    actions[unit.id] = MoveAction(collision.position, false, false);
//                    if (debugUnits.contains(unit.id)) {
//                        std::cerr << "DEBBUUUG MOVE_ACTION Collision: " << collision << ", unit_pos: " << unit.position << ", unit_id: " << unit.id << std::endl;
//                    }
//                    if (priority < 2) {
//                        debugUnits.insert(unit.id);
//                        std::cerr << "MOVE_ACTION Collision: " << collision << ", unit_pos: " << unit.position << ", unit_id: " << unit.id << std::endl;
//                    }
//                    if (priority == 10) {
//                        debugUnits.insert(unit.id);
//                        std::cerr << "prior 10 MOVE_ACTION Collision: " << collision << ", unit_pos: " << unit.position << ", unit_id: " << unit.id << std::endl;
//                    }
                    movedUnitIds.insert(unit.id);
                }
            }
        }

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
                if (stuckedUnits.contains(moveStep.target) && moveStep.target != playerView->entitiesById.at(unitId).position) {
                    continue;
                }
                moveWorld[moveStep.target.x][moveStep.target.y] = unitId;
                actions[unitId] = MoveAction(moveStep.target, false, false);
                break;
            }
            movedUnitIds.insert(unitId);
        }
    }
    for (const auto& stuckedPos : stuckedUnits) {
        std::cerr << "Tick: " << playerView->currentTick << ", stucked unit pos: " << stuckedPos << std::endl;
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

std::vector<Vec2Int> MyStrategy::getBuildingEdges(const Vec2Int& position, int size, bool areUnitsEmpty) {
    std::vector<Vec2Int> buildPositions;
    buildPositions.reserve(20);
    for (int i = 0; i < size; ++i) {
        std::vector<Vec2Int> edges = {
                {position.x - 1, position.y + i},
                {position.x + i, position.y - 1},
                {position.x + size, position.y + i},
                {position.x + i, position.y + size}
        };
        for (const auto& edge : edges) {
            if (areUnitsEmpty) {
                if (checkWorldBounds(edge.x, edge.y) && (world(edge).isEmpty() || world(edge).inEntityTypes({BUILDER_UNIT, MELEE_UNIT, RANGED_UNIT}))) {
                    buildPositions.emplace_back(edge);
                }
            } else {
                if (checkWorldBounds(edge.x, edge.y) && world(edge).isEmpty()) {
                    buildPositions.emplace_back(edge);
                }
            }
        }
    }
    return buildPositions;
}

std::vector<Vec2Int> MyStrategy::getBuildingEdges(int buildingId) {
    const auto& building = playerView->entitiesById.at(buildingId);
    int entitySize = playerView->entityProperties.at(building.entityType).size;
    return getBuildingEdges(building.position, entitySize, false);
}

void MyStrategy::setRepairBuilders(std::unordered_set<int>& busyBuilders, Actions& actions) {
    std::unordered_set<EntityType> BUILDING_TYPES = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, TURRET};
    static std::unordered_set<EntityType> OBSTACLE_TYPES = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, WALL, TURRET, RESOURCE};
    std::unordered_set<int> brokenBuildings;
    for (const Entity& base : playerView->entities) {
        if (BUILDING_TYPES.contains(base.entityType)
                && base.health < playerView->entityProperties.at(base.entityType).maxHealth
                && base.playerId == playerView->myId) {
            brokenBuildings.insert(base.id);
        }
    }

    std::unordered_map<int, int> repairersCounts;
    for (const auto& builderUnit : playerView->getMyEntities(BUILDER_UNIT)) {
        if (busyBuilders.contains(builderUnit.id)) {
            continue;
        }
        Vec2Int topPosition;
        std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
        bool isRepairer = false;
        for (const auto& edge : edges) {
            if (brokenBuildings.count(world(edge).getEntityId())) {
                busyBuilders.insert(builderUnit.id);
                topPosition = builderUnit.position;
                builderAttackActions[builderUnit.id] = RepairAction(world(edge).getEntityId());
                ++repairersCounts[world(edge).getEntityId()];
                isRepairer = true;
                break;
            } else if (world(edge).isEmpty()) {
//                const auto& edgesOfEdge = getEdges(edge);
//                for (const auto& e : edgesOfEdge) {
//                    if (brokenBuildings.count(world(e).getEntityId())) {
//                        busyBuilders.insert(builderUnit.id);
//                        topPosition = edge;
//                        isRepairer = true;
////                        actions[builderUnit.id] = MoveAction({edge.x, edge.y}, false, false);
//                        break;
//                    }
//                }
            }
        }
        if (isRepairer) {
            edges.erase(std::find(edges.begin(), edges.end(), topPosition));
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                if (enemyMap[v1.x][v1.y] <= 6) {
                    return true;
                }
                if (enemyMap[v2.x][v2.y] <= 6) {
                    return false;
                }
                return true;
            });
            edges.push_back(topPosition);
            std::reverse(edges.begin(), edges.end());
            int i = 0;
            for (const auto& edge : edges) {
                int score = 0;
                if (i++ == 0) {
                    score = 1;
                }
                if (enemyMap[edge.x][edge.y] <= 6) {
                    score = -1;
                }
                addMove(builderUnit.id, edge, score, 25);
            }
        }
    }

    std::vector<int> busyBuildersVec{busyBuilders.begin(), busyBuilders.end()};
    for (const auto& brokenBuilding : brokenBuildings) {
        const auto& building = playerView->entitiesById.at(brokenBuilding);
        if (building.active) {
            continue;
        }
        int maxBuildersCount = building.entityType == RANGED_BASE ? 10 : 4;
        std::vector<Vec2Int> buildingEdges = getBuildingEdges(brokenBuilding);
        std::vector<int> builderIds;
        const auto& bfsResult = bfs(buildingEdges, OBSTACLE_TYPES, busyBuildersVec, builderIds);
        int i = 0;

        int count = repairersCounts[brokenBuilding];
        for (int builderId : builderIds) {
            if (busyBuilders.contains(builderId)) {
                continue;
            }
            const auto& builderUnit = playerView->entitiesById.at(builderId);
            if (++count > maxBuildersCount || bfsResult[builderUnit.position.x][builderUnit.position.y] > 10) {
                break;
            }
            std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                return bfsResult[v1.x][v1.y] < bfsResult[v2.x][v2.y];
            });
            for (const auto& edge : edges) {
                addMove(builderId, edge, bfsResult[edge.x][edge.y], 20);
            }
            busyBuilders.insert(builderId);
        }
    }
}

void MyStrategy::setHouseBuilders(std::unordered_set<int>& busyBuilders, Actions& actions) {
    static std::unordered_set<EntityType> OBSTACLE_TYPES = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, WALL, TURRET, RESOURCE};
    const auto& myPlayer = playerView->playersById.at(playerView->myId);
    std::vector<PotentialBuilder> potentialBuilders;
    EntityType buildType = HOUSE;
    int buildingSize = 1;

    if (playerView->getMyEntities(RANGED_BASE).empty() && myPlayer.resource >= 470) {
        std::vector<Vec2Int> basePositions;
        for (int i = 2; i < 60; i += 1) {
            for (int j = 2; j < 60; j += 1) {
                basePositions.emplace_back(i, j);
            }
        }
        for (const auto& position : basePositions) {
            if (isEmptyForHouse(position.x, position.y, 2, busyBuilders)) {
                const PotentialBuilder& builder = calcBuildingPlaceScore(position.x, position.y, 2, busyBuilders);
                if (builder.unitId != -1) {
                    potentialBuilders.push_back(builder);
                    buildType = RANGED_BASE;
                    buildingSize = 2;
                }
            }
        }
    }
    if (potentialBuilders.empty()) {
        if (playerView->getFood() < 10 && myPlayer.resource >= 47 && playerView->getInactiveHousesCount() < 3
            && (!playerView->getMyEntities(RANGED_BASE).empty() || playerView->getMyEntities(HOUSE).size() < 4)) {
            std::vector<Vec2Int> housePositions;
//            if (playerView->getMyEntities(HOUSE).empty()) {
//                for (int i = 1; i < 60; ++i) {
//                    if (i % 3 != 1) {
//                        housePositions.emplace_back(i, 1);
//                        housePositions.emplace_back(1, i);
//                    }
//                }
//            } else {
////                housePositions.emplace_back(1, 1);
//                for (int i = edgeHousesShiftX + 3; i < 60; i += 3) {
//                    housePositions.emplace_back(i, 1);
//                }
//                for (int i = edgeHousesShiftY + 3; i < 60; i += 3) {
//                    housePositions.emplace_back(1, i);
//                }
//            }
//            for (int i = 7; i < 60; i += 5) {
//                housePositions.emplace_back(i, 6);
//                housePositions.emplace_back(6, i);
//            }
//            for (int i = 12; i < 60; i += 5) {
//                for (int j = 12; j < 60; j += 5) {
//                    housePositions.emplace_back(i, j);
//                }
//            }

            for (int i = 1; i < 60; i += 1) {
                for (int j = 1; j < 60; j += 1) {
                    housePositions.emplace_back(i, j);
                }
            }

            for (const auto& housePosition : housePositions) {
                if (isEmptyForHouse(housePosition.x, housePosition.y, 1, busyBuilders)) {
                    const PotentialBuilder& builder = calcBuildingPlaceScore(housePosition.x, housePosition.y, 1, busyBuilders);
                    if (builder.unitId != -1) {
                        potentialBuilders.push_back(builder);
                    }
                }
            }
        }
    }

    if (!potentialBuilders.empty()) {
        std::sort(potentialBuilders.begin(), potentialBuilders.end(), [&] (const PotentialBuilder& builder1, const PotentialBuilder& builder2) {
            return builder1.score < builder2.score;
//            return unit1.second.x + unit1.second.y < unit2.second.x + unit2.second.y;
        });
        const auto& potentialBuilder = potentialBuilders[0];

        const std::vector<Vec2Int>& buildingEdges = getBuildingEdges(
                {potentialBuilder.position.x - buildingSize, potentialBuilder.position.y - buildingSize},
                buildingSize * 2 + 1,
                true
        );

        if (std::find(buildingEdges.begin(), buildingEdges.end(), playerView->entitiesById.at(potentialBuilder.unitId).position) != buildingEdges.end()) {
            busyBuilders.insert(potentialBuilder.unitId);
            builderAttackActions[potentialBuilder.unitId] = BuildAction(buildType, {potentialBuilder.position.x - buildingSize,
                                                                                    potentialBuilder.position.y - buildingSize});
        } else {
            int maxBuildersCount = buildType == RANGED_BASE ? 10 : 4;
//            const std::vector<Vec2Int>& buildingEdges = getBuildingEdges(position.second, buildingSize * 2 + 1, true);
            std::vector<int> builderIds;
            const auto& bfsResult = bfs(buildingEdges, OBSTACLE_TYPES, {}, builderIds);

            int count = 0;
            for (int builderId : builderIds) {
                if (busyBuilders.contains(builderId)) {
                    continue;
                }
                const auto& builderUnit = playerView->entitiesById.at(builderId);
                if (++count > maxBuildersCount || bfsResult[builderUnit.position.x][builderUnit.position.y] > 10) {
                    break;
                }
                std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
                std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                    return bfsResult[v1.x][v1.y] < bfsResult[v2.x][v2.y];
                });
                for (const auto& edge : edges) {
                    addMove(builderId, edge, bfsResult[edge.x][edge.y], 20);
                }
                busyBuilders.insert(builderId);
            }
        }
    }
}

void MyStrategy::setRunningFromEnemyBuilders(std::unordered_set<int>& busyBuilders, Actions& actions) {
    for (const auto& builderUnit : playerView->getMyEntities(BUILDER_UNIT)) {
        if (busyBuilders.contains(builderUnit.id)) {
            continue;
        }
        bool isAfraidEnemy = false;
        for (const DistId &distId : myToEnemyMapping.at(builderUnit.id)) {
            const auto &enemy = playerView->entitiesById.at(distId.entityId);
            if (enemy.entityType == RANGED_UNIT && distId.distance <= 7
                    || enemy.entityType == MELEE_UNIT && distId.distance <= 3) {
                std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
                std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                    return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                });
                std::reverse(edges.begin(), edges.end());
                for (const auto& edge : edges) {
                    addMove(builderUnit.id, edge, enemyMap[edge.x][edge.y], 1);
                }
                busyBuilders.insert(builderUnit.id);
                isAfraidEnemy = true;
            }
            if (isAfraidEnemy || distId.distance > 7) {
                break;
            }
        }
    }
}

void MyStrategy::setAttackBuilders(std::unordered_set<int> &busyBuilders, Actions &actions) {
    static std::unordered_set<EntityType> typesToAttack = {BUILDER_UNIT, HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, WALL};
    for (const auto& builderUnit : playerView->getMyEntities(BUILDER_UNIT)) {
        if (busyBuilders.contains(builderUnit.id)) {
            continue;
        }
        for (const auto& edge : edgesMap[builderUnit.position.x][builderUnit.position.y]) {
            if (!world(edge).eqPlayerId(playerView->myId) && world(edge).inEntityTypes(typesToAttack)) {
                busyBuilders.insert(builderUnit.id);
                builderAttackActions[builderUnit.id] = AttackAction(world(edge).getEntityId());
                break;
            }
        }
    }
}

void MyStrategy::setFarmers(std::unordered_set<int>& busyBuilders, Actions& actions) {
    for (const auto& builderUnit : playerView->getMyEntities(BUILDER_UNIT)) {
        if (busyBuilders.contains(builderUnit.id)) {
            continue;
        }
        std::vector<Vec2Int> edges = getEdges(builderUnit.position, false);
        bool isFarmer = false;
        for (const auto& edge : edges) {
            if (world(edge).eqEntityType(RESOURCE)) {
                busyBuilders.insert(builderUnit.id);
                builderAttackActions[builderUnit.id] = AttackAction(world(edge).getEntityId());
                isFarmer = true;
                break;
            }
        }

        if (isFarmer) {
            edges.push_back(builderUnit.position);
            std::reverse(edges.begin(), edges.end());
            int i = 0;
            for (const auto& edge : edges) {
                int score = 0;
                if (i++ == 0) {
                    score = 1;
                }
                addMove(builderUnit.id, edge, score, 25);
            }
        }
    }
}

void MyStrategy::setMovingToFarm(std::unordered_set<int>& busyBuilders, Actions& actions) {
    static std::unordered_set<EntityType> obstacleTypes = {HOUSE, BUILDER_BASE, RANGED_BASE, MELEE_BASE, WALL, TURRET, RESOURCE};
    std::vector<EntityPtr> builders;
    std::vector<Vec2Int> buildersPositions;
    for (const auto& builderUnit : playerView->getMyEntities(BUILDER_UNIT)) {
        if (busyBuilders.contains(builderUnit.id)) {
            continue;
        }
        builders.push_back(&builderUnit);
        buildersPositions.push_back(builderUnit.position);
    }

    const auto& bfsResult = bfsBuilderResources({{5, 5}}, obstacleTypes, busyBuilders);
    std::unordered_set<Vec2Int> farmTargets{bfsResult.begin(), bfsResult.end()};

    std::unordered_map<EntityPtr, int> buildersToTargetDist;
    for (EntityPtr builder : builders) {
        int minDist = 100000;
        for (const auto& farmTarget : farmTargets) {
            int distance = dist(builder->position, farmTarget);
            if (distance < minDist) {
                minDist = distance;
            }
        }
        buildersToTargetDist[builder] = minDist;
    }
    std::sort(builders.begin(), builders.end(), [&](EntityPtr b1, EntityPtr b2) {
        return buildersToTargetDist.at(b1) < buildersToTargetDist.at(b2);
    });

    std::unordered_map<EntityPtr, Vec2Int> buildersToTarget;
    for (EntityPtr builder : builders) {
        if (farmTargets.empty()) {
            break;
        }
        Vec2Int closestResource;
        int minDist = 100000;
        for (const auto& farmTarget : farmTargets) {
            int distance = dist(builder->position, farmTarget);
            if (distance < minDist) {
                minDist = distance;
                closestResource = farmTarget;
            }
        }
        buildersToTarget[builder] = closestResource;
        farmTargets.erase(closestResource);
    }
    farmTargets_ = {farmTargets.begin(), farmTargets.end()};

    std::unordered_map<Vec2Int, std::array<std::array<int, 80>, 80>> bfsResults;
    for (const auto& [builderPtr, target] : buildersToTarget) {
        Vec2Int targetPosition = target;
        if (!bfsResults.contains(target)) {
            bfsResults[target] = bfs({target}, obstacleTypes, {});
        }
        std::vector<Vec2Int> edges = getEdges(builderPtr->position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return bfsResults.at(targetPosition)[v1.x][v1.y] < bfsResults.at(targetPosition)[v2.x][v2.y];
        });
        for (const auto& edge : edges) {
            addMove(builderPtr->id, edge, bfsResults.at(target)[edge.x][edge.y], 20);
        }
    }

}

void MyStrategy::setBuilderUnitsActions(Actions& actions) {
    std::unordered_set<int> busyBuilders;
    setRepairBuilders(busyBuilders, actions);
    setHouseBuilders(busyBuilders, actions);
    setRunningFromEnemyBuilders(busyBuilders, actions);
    setAttackBuilders(busyBuilders, actions);
    setFarmers(busyBuilders, actions);
    setMovingToFarm(busyBuilders, actions);
}

void MyStrategy::handleBuilderAttackActions(Actions &actions) {
    for (const auto& [unitId, attackAction] : builderAttackActions) {
        if (!actions.contains(unitId) || actions[unitId].moveAction->target == playerView->entitiesById.at(unitId).position) {
            actions[unitId] = attackAction;
        }
    }
    builderAttackActions.clear();
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

std::ostream& operator<<(std::ostream& out, const Vec2Int& cell) {
    out << "{ x: " << cell.x << ", y: " << cell.y << " }";
    return out;
}

std::ostream& operator<<(std::ostream& out, const CollisionPriority& collisionPriority) {
    out << "{ target: " << collisionPriority.position << ", units: " << collisionPriority.units.size() << " }";
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
