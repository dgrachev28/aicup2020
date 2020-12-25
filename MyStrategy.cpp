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

std::array<std::array<BfsBuilding, 80>, 80>
MyStrategy::bfs(const std::vector<Vec2Int>& startCells,
                const std::unordered_set<EntityType>& obstacleTypes,
                const std::vector<int>& obstacleUnitIds,
                std::vector<int>& closestUnits) {
    std::array<std::array<BfsBuilding, 80>, 80> d;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            d[i][j] = {{0, 0}, 100000};
        }
    }
    std::queue<Vec2Int> q;
    for (const Vec2Int& cell : startCells) {
        if (std::find(obstacleUnitIds.begin(), obstacleUnitIds.end(), world(cell).getEntityId()) == obstacleUnitIds.end()) {
            d[cell.x][cell.y] = {cell, 0};
            q.push(cell);
        }
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
//        if (!closestUnits.empty()) {
//            continue;
//        }
        if (closestUnits.empty()
                && world(v).eqEntityType(BUILDER_UNIT)
                && world(v).eqPlayerId(playerView->myId)
                && std::find(obstacleUnitIds.begin(), obstacleUnitIds.end(), world(v).getEntityId()) == obstacleUnitIds.end()) {
            closestUnits.push_back(world(v).getEntityId());
        }
        for (const Vec2Int& edge : edgesMap[v.x][v.y]) {
            if (d[edge.x][edge.y].score > d[v.x][v.y].score + 1
                    && !world(edge).inEntityTypes(obstacleTypes)) {
                d[edge.x][edge.y].score = d[v.x][v.y].score + 1;
                d[edge.x][edge.y].position = d[v.x][v.y].position;
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
int MyStrategy::bfsForValidateBuildings(const std::vector<Vec2Int>& startCells,
                                        const std::unordered_set<EntityType>& obstacleTypes = {},
                                        const std::unordered_set<Vec2Int>& obstacleCells = {}) {
    int visitedCellsCount = 0;
    static std::unordered_set<EntityType> UNIT_TYPES = {BUILDER_UNIT, RANGED_UNIT, MELEE_UNIT};
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
        ++visitedCellsCount;
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1
                    && d[v.x][v.y] <= 10
                    && !world(edge).inEntityTypes(obstacleTypes)
                    && !obstacleCells.contains(v)) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }
    return visitedCellsCount;
}


std::vector<Vec2Int> MyStrategy::bfsBuilderResources(const std::vector<Vec2Int>& startCells,
                                                     const std::unordered_set<EntityType>& obstacleTypes = {},
                                                     const std::unordered_set<Vec2Int>& obstacleCells = {}) {
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
                    && !obstacleCells.contains(v)) {
                pushed[v.x][v.y] = true;
                builderResourcesCells.push_back(v);
            }
        }
    }
    return builderResourcesCells;
}


std::array<std::array<int, 80>, 80> MyStrategy::dijkstra(const std::vector<Vec2Int>& startCells, bool isWeighted = true) {
    static std::unordered_set<EntityType> BUILDINGS = {HOUSE, BUILDER_BASE, MELEE_BASE, RANGED_BASE, TURRET};
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
                        len = 18;
                    }
                    if (type == RANGED_UNIT && enemyMap[edge.x][edge.y] <= 6) {
                        len = 200;
                    }
                    if (type == RANGED_UNIT && enemyMap[edge.x][edge.y] == 7) {
                        len = 200;
                    }
                    if (type == BUILDER_UNIT && world(edge).farmBuilder) {
                        len = 70;
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

int dist2(const Vec2Int& p1, const Vec2Int& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
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

Action MyStrategy::getAction(PlayerView& playerView, DebugInterface* debugInterface) {
    this->playerView = &playerView;
    addPreviousStepInfo(playerView);
    stepInit(playerView);

    std::unordered_map<int, EntityAction> actions;

    setBuilderUnitsActions(actions);
    getRangedUnitAction(playerView, actions);
    getBuildUnitActions(playerView, actions);

    shootResources(actions);
    handleMoves(actions, movePriorityToUnitIds, unitMoveSteps);
    handleAttackActions(actions);
    handleBuilderAttackActions(actions);
    shootResourcesAgain(actions);
    saveStepState();

    visionBounds.clear();
    unitMoveSteps.clear();
    movePriorityToUnitIds.clear();
    shootResourcePositions.clear();

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
    std::vector<Vec2Int> resourcesBfsPositions;
    std::vector<Vec2Int> myBuildingsBfsPositions;
    std::vector<Vec2Int> myBuildersBfsPositions;
    for (auto& entity : playerView.entities) {
        const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);
        for (int i = entity.position.x; i < entity.position.x + properties.size; ++i) {
            for (int j = entity.position.y; j < entity.position.y + properties.size; ++j) {
                world(i, j).entity = &entity;
            }
        }
        if ((entity.entityType == HOUSE || entity.entityType == RANGED_BASE || entity.entityType == TURRET)
                && entity.playerId == playerView.myId && !entity.active) {
            for (const auto& buildingEdge : getBuildingEdges(entity.id)) {
                myBuildingsBfsPositions.push_back(buildingEdge);
            }
        }
        if (entity.entityType == BUILDER_UNIT && entity.playerId == playerView.myId) {
            myBuildersBfsPositions.push_back(entity.position);
            for (const auto& edge : edgesMap[entity.position.x][entity.position.y]) {
                if (world(edge).eqEntityType(RESOURCE)) {
                    world(entity.position).farmBuilder = true;
                    break;
                }
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
            if (myRangedCount < 8) {
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

    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            if (world(i, j).isEmpty()) {
                for (const auto& edge : edgesMap[i][j]) {
                    if (world(edge).eqEntityType(RESOURCE)) {
                        resourcesBfsPositions.emplace_back(i, j);
                        break;
                    }
                }
            }
        }
    }

    const auto& myBuildingsBfs = bfs(myBuildingsBfsPositions);
    const auto& myBuildersBfs = bfs(myBuildersBfsPositions);
    const auto& resourcesBfs = bfs(resourcesBfsPositions);
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            world(i, j).myBuildingsBfs = myBuildingsBfs[i][j];
            world(i, j).myBuildersBfs = myBuildersBfs[i][j];
            world(i, j).resourcesBfs = resourcesBfs[i][j];
        }
    }

    enemyPossibleMoves = getNextEnemyMoves();
}

void MyStrategy::addPreviousStepInfo(PlayerView& playerView) {
    if (previousEntities.empty() || !playerView.fogOfWar) {
        return;
    }

    std::array<std::array<int, 80>, 80> d;
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            d[i][j] = 100000;
        }
    }
    std::queue<Vec2Int> q;
    for (const auto& entity : playerView.entities) {
        if (entity.playerId == playerView.myId
                && (entity.entityType == BUILDER_UNIT
                || entity.entityType == MELEE_UNIT
                || entity.entityType == RANGED_UNIT)) {
            d[entity.position.x][entity.position.y] = 0;
            q.push(entity.position);
        }
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
        for (const Vec2Int& edge : edgesMap[v.x][v.y]) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1 && d[v.x][v.y] < 10) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }

    for (const auto& entity : playerView.entities) {
        if (entity.playerId == playerView.myId
                && (entity.entityType == BUILDER_BASE
                || entity.entityType == MELEE_BASE
                || entity.entityType == RANGED_BASE
                || entity.entityType == HOUSE)) {
            const EntityProperties &properties = playerView.entityProperties.at(entity.entityType);
            for (int i = entity.position.x; i < entity.position.x + properties.size; ++i) {
                for (int j = entity.position.y; j < entity.position.y + properties.size; ++j) {
                    d[i][j] = 0;
                    q.push({i, j});
                }
            }
        }
    }

    while (!q.empty()) {
        const Vec2Int& v = q.front();
        q.pop();
        for (const Vec2Int& edge : edgesMap[v.x][v.y]) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1 && d[v.x][v.y] < 5) {
                d[edge.x][edge.y] = d[v.x][v.y] + 1;
                q.push(edge);
            }
        }
    }

    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 80; ++j) {
            if (d[i][j] != 100000) {
                continue;
            }
            for (const auto& edge : edgesMap[i][j]) {
                if (d[edge.x][edge.y] != 100000 && (edge.x > i || edge.y > j)) {
                    visionBounds.emplace_back(i, j);
                }
            }
        }
    }


    for (const auto& entity : previousEntities) {
        if (playerView.entitiesById.contains(entity.id) || entity.playerId == playerView.myId) {
            continue;
        }
        const EntityProperties &properties = playerView.entityProperties.at(entity.entityType);
        for (int i = entity.position.x; i < entity.position.x + properties.size; ++i) {
            bool stopLoop = false;
            for (int j = entity.position.y; j < entity.position.y + properties.size; ++j) {
                if (d[i][j] == 100000) {
                    playerView.entities.push_back(entity);
                    playerView.entitiesById[entity.id] = entity;
                    playerView.entitiesByPlayerId[entity.playerId][entity.entityType].push_back(entity);
//                    if (entity.entityType == RANGED_UNIT || entity.entityType == BUILDER_UNIT || entity.entityType == HOUSE) {
//                        std::cerr << "tick: " << playerView.currentTick << ", pos: " << entity.position << std::endl;
//                    }
                    stopLoop = true;
                    break;
                }
            }
            if (stopLoop) {
                break;
            }
        }
    }

}

void MyStrategy::saveStepState() {
    previousEntities = playerView->entities;
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
                maxBuildersCount = 60;
//            } else if (playerView.playersById.at(playerView.myId).resource < 1000) {
//                maxRangedCount = 45;
//                maxBuildersCount = 80;
            } else {
                maxRangedCount = 100;
                maxBuildersCount = 80;
            }
            if (playerView.getMyEntities(RANGED_UNIT).size() >= 40) {
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
                maxRangedCount = 100;
                maxBuildersCount = 60;
            }
        }
        if (playerView.currentTick > 600) {
            maxBuildersCount = (1000 - playerView.currentTick) / (1000 - 600);
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
    const auto& startCells = getBuildingEdges({x - size, y - size}, size * 2 + 1, {MELEE_UNIT, RANGED_UNIT, BUILDER_UNIT});
    int maxUnitsCount = size == 2 ? 12 : 5;
    int maxEmptyDistance = size == 2 ? 7 : 5;
    int maxDistance = 10;

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
        if (world(v).eqEntityType(BUILDER_UNIT) && world(v).eqPlayerId(playerView->myId) && !busyBuilders.contains(world(v).getEntityId())) {
            closestUnits.push_back({d[v.x][v.y], v, world(v).getEntityId()});
        }
        if (closestUnits.size() >= maxUnitsCount || (closestUnits.empty() && d[v.x][v.y] > maxEmptyDistance) || d[v.x][v.y] > maxDistance) {
            break;
        }
        const std::vector<Vec2Int>& edges = edgesMap[v.x][v.y];
        for (const Vec2Int& edge : edges) {
            if (d[edge.x][edge.y] > d[v.x][v.y] + 1
                    && !world(edge).inEntityTypes(obstacleTypes)) {
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
    for (int i = closestUnits.size(); i < maxUnitsCount; ++i) {
        score += maxDistance;
    }
    score /= maxUnitsCount;
//    score *= 2;
//    score += static_cast<float>(x + y);
    return {closestUnits[0].entityId, score, {x, y}};
}

bool MyStrategy::isEmptyForHouse(int x, int y, int size, const std::unordered_set<int>& busyBuilders) {
    static std::unordered_set<EntityType> BUILDINGS = {HOUSE, MELEE_BASE, RANGED_BASE, TURRET};
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
    std::unordered_map<int, Vec2Int> defenderScouts;
    if (playerView.fogOfWar) {
        if (scouts.empty() && freeScoutSpots.empty()) {
            if (isFinal) {
                freeScoutSpots = {{70, 70}};
            } else {
                freeScoutSpots = {{9, 70}, {70, 9}, {70, 70}};
            }
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

        std::sort(visionBounds.begin(), visionBounds.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return world(v1).myBuildersBfs < world(v2).myBuildersBfs;
        });
        for (const auto& visionBound : visionBounds) {
            if (visionBound.x + visionBound.y < 20) {
                continue;
            }
            bool isTooClose = false;
            for (const auto& [unitId, position] : defenderScouts) {
                if (dist(position, visionBound) < 20) {
                    isTooClose = true;
                    break;
                }
            }
            if (!isTooClose) {
                int minDist = 10000;
                int closestUnitId = -1;
                for (const auto& unit : playerView.getMyEntities(RANGED_UNIT)) {
                    int distance = dist(unit.position, visionBound);
                    if (!defenderScouts.contains(unit.id) && distance < minDist) {
                        minDist = distance;
                        closestUnitId = unit.id;
                    }
                }
                if (closestUnitId != -1) {
//                    std::cerr << "tick: " << playerView.currentTick << ", unitId: " << closestUnitId
//                              << ", postion: " << visionBound << ", ranged dist: " << minDist
//                              << ", builder dist: " << world(visionBound).myBuildersBfs << std::endl;
                    defenderScouts[closestUnitId] = visionBound;
                }
                if (defenderScouts.size() >= 3 || world(visionBound).myBuildersBfs > 20) {
                    break;
                }
            }
        }
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
                if (i >= potential.count + 1) {
                    break;
                }
            }
        }
    }

    std::unordered_map<int, Vec2Int> attackers;
    for (const auto& potential : topAttackPotentials) {
        if (world(potential.x, potential.y).isEmpty()) {
            continue;
        }
        if (!dijkstraResults.count({potential.x, potential.y})) {
            dijkstraResults[{potential.x, potential.y}] = dijkstra({{potential.x, potential.y}});
        }
        int i = 0;
        for (const DistId& distId : enemyToMyMapping[world(potential.x, potential.y).entity->id]) {
            const Entity& myUnit = playerView.entitiesById.at(distId.entityId);
            if (myUnit.entityType == RANGED_UNIT && !defenders.count(myUnit.id) && !attackers.count(myUnit.id)) {
                attackers[myUnit.id] = {potential.x, potential.y};
                ++i;
                if (i >= 3) {
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
        if (playerView.currentTick > 100 || minEnemyDist < 30) {
            if (defenders.count(unit.id)) {
                targetPosition = defenders[unit.id];
            } else if (attackers.count(unit.id)) {
                targetPosition = attackers[unit.id];
            } else {
                if (playerView.fogOfWar) {
                    if (scouts.contains(unit.id)) {
                        targetPosition = scouts[unit.id];
                    }
                    if (defenderScouts.contains(unit.id)) {
                        targetPosition = defenderScouts[unit.id];
                    }
                }
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
            dijkstraResults[targetPosition] = dijkstra({targetPosition});
        }
        std::vector<Vec2Int> edges = getEdges(unit.position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            int score1 = dijkstraResults.at(targetPosition)[v1.x][v1.y] * 1000 + dist2(v1, targetPosition);
            int score2 = dijkstraResults.at(targetPosition)[v2.x][v2.y] * 1000 + dist2(v2, targetPosition);
            return score1 < score2;
        });

        if (world(edges[0]).eqEntityType(RESOURCE)) {
            builderAttackActions[unit.id] = AttackAction(world(edges[0]).getEntityId());
            shootResourcePositions.push_back(edges[0]);
        }
        for (const auto& edge : edges) {
            addMove(unit.id, edge, dijkstraResults.at(targetPosition)[edge.x][edge.y] * 1000 + dist2(edge, targetPosition), 10);
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

            if (i > 0) {
                avgBuilderDist /= static_cast<float>(i);
            }
            float myBuildingScore = customDecay(buildingDist);
            float myBuilderScore = customDecay(avgBuilderDist);

            Score score{myBuildingScore, myBuilderScore, myPowerScore, enemyPowerScore};
//            score.calcAttackScore();
            float myDistScore = customDecay(myMap[enemy.position.x][enemy.position.y]);
            float enemyDistScore = customDecay(enemyMap[enemy.position.x][enemy.position.y]);
            score.score = myDistScore - enemyDistScore;
            attackPotentials.push_back({score, enemy.position.x, enemy.position.y, 1});
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
        if (topPotentials.size() >= 8 || p.score.score < 50) {
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
    std::cerr << "================== DEFENSE ===================" << std::endl;
    for (const auto& tp : topPotentials) {
        std::cerr << "tick: " << playerView.currentTick << ", score: " << tp << std::endl;
    }
    std::cerr << "================== ATTACK ====================" << std::endl;
    for (const auto& tp : topAttackPotentials) {
        std::cerr << "tick: " << playerView.currentTick << ", score: " << tp << std::endl;
    }
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
        int myPower = my0 * 3 + my1 * 3 + my2;
        int enemyPower = enemy0 * 3 + enemy1 * 3 + enemy2;
//        MicroState microState;
//        if (minDefenseDist < 10) {
//            if (myPower >= enemyPower) {
//                microState = MicroState::ATTACK;
//            } else if (myPower >= enemyPower - 3) {
//                microState = MicroState::STAY;
//            } else {
//                microState = MicroState::RUN_AWAY;
//            }
//        } else {
//            if (myPower >= enemyPower + 3) {
//                microState = MicroState::ATTACK;
//            } else if (myPower > enemyPower) {
//                microState = MicroState::STAY;
//            } else {
//                microState = MicroState::RUN_AWAY;
//            }
//        }
        const MicroState& microState = simulateBattle(group, minDefenseDist < 10);


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

                    addMove(unit.id, unit.position, 10, 2);
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
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 7) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, true);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 2);
                    }
                }
            }
        } else if (microState == MicroState::ATTACK) {
            for (const auto& [unitId, distance] : group) {
                const Entity& unit = playerView->entitiesById.at(unitId);
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 6) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, true);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 1);
                    }
                }
                if (unit.playerId == playerView->myId && enemyMap[unit.position.x][unit.position.y] == 7) {
                    std::vector<Vec2Int> edges = getEdges(unit.position, true);
                    std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                        return enemyMap[v1.x][v1.y] < enemyMap[v2.x][v2.y];
                    });
                    for (const auto& edge : edges) {
                        addMove(unit.id, edge, enemyMap[edge.x][edge.y], 2);
                    }
                }
            }
        }
    }
}

MicroState MyStrategy::simulateBattle(const std::unordered_map<int, int>& group, bool defense) {
    Simulation attackSim{};
    Simulation staySim{};
    Simulation runAwaySim{};
    Simulation enemyAttackSim{};
    Simulation enemyStaySim{};
//    Simulation enemyRunAwaySim{};

    for (const auto& [unitId, distance] : group) {
        const Entity& unit = playerView->entitiesById.at(unitId);
        if (unit.playerId == playerView->myId) {
            microAttack(unit, 6, 1, enemyMap, attackSim);
            microAttack(unit, 7, 2, enemyMap, attackSim);
            microStay(unit, 6, 1, enemyMap, staySim);
            microAttack(unit, 7, 2, enemyMap, staySim);
            microRunAway(unit, 6, 1, enemyMap, runAwaySim);
            microStay(unit, 7, 2, enemyMap, runAwaySim);
        } else {
            microAttack(unit, 6, 1, myMap, enemyAttackSim);
            microAttack(unit, 7, 2, myMap, enemyAttackSim);
            microStay(unit, 6, 1, myMap, enemyStaySim);
            microAttack(unit, 7, 2, myMap, enemyStaySim);
//            microRunAway(unit, 6, 1, myMap, enemyRunAwaySim);
//            microStay(unit, 7, 2, myMap, enemyRunAwaySim);
        }
    }
    std::vector<Simulation*> mySims{&attackSim, &staySim, &runAwaySim};
    std::vector<Simulation*> enemySims{&enemyAttackSim, &enemyStaySim};
    for (Simulation* sim : mySims) {
        handleMoves(sim->myMoves, sim->movePriorityToUnitIds, sim->unitMoveSteps);
    }
    for (Simulation* sim : enemySims) {
        handleMoves(sim->myMoves, sim->movePriorityToUnitIds, sim->unitMoveSteps);
    }
    std::vector<int> scores;
    std::vector<int> anyDeaths;
    for (Simulation* mySim : mySims) {
        bool anyDeath1, anyDeath2, anyDeath;
        int score1 = calculateSimScore(*mySim, *enemySims[0], anyDeath1);
        int score2 = calculateSimScore(*mySim, *enemySims[1], anyDeath2);
        int score;
        if (score1 < score2) {
            score = score1;
            anyDeath = anyDeath1;
        }
        if (score1 > score2) {
            score = score2;
            anyDeath = anyDeath2;
        }
        if (score1 == score2) {
            score = score1;
            anyDeath = anyDeath1 || anyDeath2;
        }
//        std::cerr << "tick: " << playerView->currentTick << ", score: " << score << ", group size: " << group.size() << std::endl;
        scores.push_back(score);
        anyDeaths.push_back(anyDeath);
    }
    if (scores[0] > scores[1] && scores[0] > scores[2]) {
        return MicroState::ATTACK;
    }
    if (scores[1] > scores[0] && scores[1] > scores[2]) {
        return MicroState::STAY;
    }
    if (scores[2] > scores[0] && scores[2] > scores[1]) {
        return MicroState::RUN_AWAY;
    }
    int maxScore = *std::max_element(scores.begin(), scores.end());
//    std::cerr << "tick: " << playerView->currentTick << ", max score: " << maxScore << std::endl;
    if (maxScore > 0) {
        if (scores[1] == maxScore) {
            return MicroState::STAY;
        } else {
            return MicroState::ATTACK;
        }
    } if (maxScore == 0) {
        if (scores[0] == maxScore && (!anyDeaths[0] || defense)) {
            return MicroState::ATTACK;
        }
        if (scores[1] == maxScore && !anyDeaths[1]) {
            return MicroState::STAY;
        }
        if (scores[2] == maxScore) {
            return MicroState::RUN_AWAY;
        }
        return MicroState::STAY;
    }

    return MicroState::RUN_AWAY;
}

void MyStrategy::microAttack(const Entity& unit, int enemyMapDist, int priority, const std::array<std::array<int, 80>, 80>& battleMap, Simulation& sim) {
    if (battleMap[unit.position.x][unit.position.y] == enemyMapDist) {
        std::vector<Vec2Int> edges = getEdges(unit.position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return battleMap[v1.x][v1.y] < battleMap[v2.x][v2.y];
        });
        for (const auto& edge : edges) {
            addMoveSim(unit.id, edge, battleMap[edge.x][edge.y], priority, sim);
        }
    }
}

void MyStrategy::microStay(const Entity& unit, int enemyMapDist, int priority, const std::array<std::array<int, 80>, 80>& battleMap, Simulation& sim) {
    if (battleMap[unit.position.x][unit.position.y] == enemyMapDist) {
        std::vector<Vec2Int> edges = getEdges(unit.position, false);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return battleMap[v1.x][v1.y] < battleMap[v2.x][v2.y];
        });
        std::reverse(edges.begin(), edges.end());

        addMoveSim(unit.id, unit.position, 10, priority, sim);
        for (const auto& edge : edges) {
            addMoveSim(unit.id, edge, battleMap[edge.x][edge.y], priority, sim);
        }
    }
}
void MyStrategy::microRunAway(const Entity& unit, int enemyMapDist, int priority, const std::array<std::array<int, 80>, 80>& battleMap, Simulation& sim) {
    if (battleMap[unit.position.x][unit.position.y] == enemyMapDist) {
        std::vector<Vec2Int> edges = getEdges(unit.position, true);
        std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
            return battleMap[v1.x][v1.y] < battleMap[v2.x][v2.y];
        });
        std::reverse(edges.begin(), edges.end());
        for (const auto& edge : edges) {
            addMoveSim(unit.id, edge, battleMap[edge.x][edge.y], priority, sim);
        }
    }
}

std::unordered_map<int, std::vector<Vec2Int>> MyStrategy::getNextEnemyMoves() {
    static std::unordered_set<EntityType> UNIT_TYPES = {BUILDER_UNIT, MELEE_UNIT, RANGED_UNIT};
    std::unordered_map<int, std::vector<Vec2Int>> enemyMoves;

    for (const auto& unit : playerView->entities) {
        if (unit.entityType == RANGED_UNIT && unit.playerId != playerView->myId) {
            std::vector<Vec2Int> enemyEdges;
            if (myMap[unit.position.x][unit.position.y] <= 5) {
                enemyEdges.push_back(unit.position);
            } else {
                for (const auto& edge : getEdges(unit.position, true)) {
                    if (world(edge).isEmpty() || world(edge).inEntityTypes(UNIT_TYPES)) {
                        enemyEdges.push_back(edge);
                    }
                }
            }
            enemyMoves[unit.id] = enemyEdges;
        }
    }
    return enemyMoves;
}

int MyStrategy::calculateSimScore(const Simulation& mySim, const Simulation& enemySim, bool& anyDeath) {
    std::unordered_set<int> battleEnemyIds;
    std::unordered_set<int> battleMyIds;
    for (const auto& [myUnitId, myAction] : mySim.myMoves) {
        if (!myAction.moveAction) {
            continue;
        }
        for (const auto& [enemyUnitId, enemyAction] : enemySim.myMoves) {
            if (enemyAction.moveAction && dist(enemyAction.moveAction->target, myAction.moveAction->target) <= 5) {
                battleEnemyIds.insert(enemyUnitId);
                battleMyIds.insert(myUnitId);
            }
        }
    }
//    std::cerr << "tick: " << playerView->currentTick << ", my score: " << battleMyIds.size()
//              << ", enemy size: " << battleEnemyIds.size() << std::endl;
    anyDeath = battleMyIds.size() + battleEnemyIds.size() > 0;
    return battleMyIds.size() - battleEnemyIds.size();
}

void MyStrategy::calculateSimulationScore(Simulation& sim) {
    std::unordered_set<Vec2Int> battleEnemyPositions;
    std::unordered_set<int> battleEnemyIds;
    std::unordered_set<int> battleMyIds;

    for (const auto& [enemyId, enemyMoves] : enemyPossibleMoves) {
        int minMyCount = 10000;
        std::vector<PositionCount> enemyDistPositions;
        for (const Vec2Int& enemyMove : enemyMoves) {
            int count = 0;
            for (const auto& [myUnitId, myAction] : sim.myMoves) {
                if (myAction.moveAction && dist(enemyMove, myAction.moveAction->target) <= 5) {
                    ++count;
                }
            }
            enemyDistPositions.push_back({enemyMove, count});
        }
        std::sort(enemyDistPositions.begin(), enemyDistPositions.end(), [](const PositionCount& p1, const PositionCount& p2) {
            if (p1.count == 0) {
                return false;
            }
            if (p2.count == 0) {
                return true;
            }
            return p1.count < p2.count;
        });
        std::optional<int> prevCount;
        for (const PositionCount& posCount : enemyDistPositions) {
            if (!prevCount || *prevCount == posCount.count) {
                for (const auto& [myUnitId, myAction] : sim.myMoves) {
                    if (myAction.moveAction && dist(posCount.position, myAction.moveAction->target) <= 5) {
                        battleEnemyPositions.insert(posCount.position);
                        battleEnemyIds.insert(enemyId);
                        battleMyIds.insert(myUnitId);
                    }
                }
            }
            prevCount = posCount.count;
        }
    }
//    for (const auto& [myUnitId, myAction] : sim.myMoves) {
//        if (!myAction.moveAction) {
//            continue;
//        }
//        for (const auto& [enemyId, enemyMoves] : enemyPossibleMoves) {
//            for (const Vec2Int& enemyMove : enemyMoves) {
//                if (dist(enemyMove, myAction.moveAction->target) <= 5) {
//                    battleEnemyPositions.insert(enemyMove);
//                    battleEnemyIds.insert(enemyId);
//                    battleMyIds.insert(myUnitId);
//                }
//            }
//        }
//    }
    int myPower = battleMyIds.size();
    int enemyPower = std::min(battleEnemyIds.size(), battleEnemyPositions.size());
    std::cerr << "tick: " << playerView->currentTick << ", my score: " << myPower
              << ", enemy size: " << battleEnemyIds.size()
              << ", enemy positions: " << battleEnemyPositions.size() << std::endl;
    sim.score = myPower - enemyPower;
}

void MyStrategy::addMoveSim(int unitId, const Vec2Int& target, int score, int priority, Simulation& sim) {
    if (!sim.movePriorityToUnitIds[priority].count(unitId)) {
        sim.unitMoveSteps[unitId].clear();
    }
    sim.movePriorityToUnitIds[priority].insert(unitId);
    sim.unitMoveSteps[unitId].push_back({unitId, target, score});
}


void MyStrategy::shootResources(Actions& actions) {
    for (const auto& unit : playerView->getMyEntities(RANGED_UNIT)) {
        if (enemyMap[unit.position.x][unit.position.y] < 8) {
            continue;
        }
        for (const auto& pos : shootResourcePositions) {
            if (dist(pos, unit.position) <= 5) {
                std::vector<MoveStep> newMoveSteps;
                newMoveSteps.push_back({unit.id, unit.position, 0});
                for (const auto& moveStep : unitMoveSteps[unit.id]) {
                    if (moveStep.target != unit.position) {
                        newMoveSteps.push_back(moveStep);
                    }
                }
                unitMoveSteps[unit.id] = newMoveSteps;
                builderAttackActions[unit.id] = AttackAction(world(pos).getEntityId());
            }
        }
    }
}

void MyStrategy::shootResourcesAgain(Actions& actions) {
    for (const auto& unit : playerView->getMyEntities(RANGED_UNIT)) {
        if (!actions[unit.id].attackAction && (!actions[unit.id].moveAction || actions[unit.id].moveAction->target == unit.position)) {
            for (const auto& pos : shootResourcePositions) {
                if (dist(pos, unit.position) <= 5) {
                    std::vector<MoveStep> newMoveSteps;
                    newMoveSteps.push_back({unit.id, unit.position, 0});
                    for (const auto& moveStep : unitMoveSteps[unit.id]) {
                        if (moveStep.target != unit.position) {
                            newMoveSteps.push_back(moveStep);
                        }
                    }
                    unitMoveSteps[unit.id] = newMoveSteps;
                    builderAttackActions[unit.id] = AttackAction(world(pos).getEntityId());
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

// return moveWorld
// param movePriorityToUnitIds
// param unitMoveSteps
// param actions&

void MyStrategy::handleMoves(
        Actions& actions,
        std::map<int, std::unordered_set<int>>& movePriorityToUnitIds,
        std::unordered_map<int, std::vector<MoveStep>>& unitMoveSteps
) {
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
    for (const auto& [priority, unitIds] : movePriorityToUnitIds) {
        std::queue<int> unitsQueue;
        for (int unitId : unitIds) {
            int currentPosMovedUnitId = moveWorld[playerView->entitiesById.at(unitId).position.x][playerView->entitiesById.at(unitId).position.y];
            if (currentPosMovedUnitId >= 0) {
                unitsQueue.push(unitId);
            }
        }
        while (!unitsQueue.empty()) {
            int unitId = unitsQueue.front();
//            if (priority == 10) {
//                std::cerr << "Tick: " << playerView->currentTick << ", unitId: " << unitId << std::endl;
//            }
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
                moveWorld[moveStep.target.x][moveStep.target.y] = unitId;
                actions[unitId] = MoveAction(moveStep.target, false, false);
//                if (priority == 10) {
//                    std::cerr << "prior 10 QUEUE move: " << moveStep.target << ", unit_pos: " << playerView->entitiesById.at(unitId).position << ", unit_id: " << unitId << std::endl;
//                }
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
//                if (priority < 2) {
//                    std::cerr << "Tick: " << playerView->currentTick << ", collision: " << collision << std::endl;
//                }
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
                moveWorld[moveStep.target.x][moveStep.target.y] = unitId;
                actions[unitId] = MoveAction(moveStep.target, false, false);
                break;
            }
            movedUnitIds.insert(unitId);
        }
    }
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
                if ((enemyHealth[enemy1] + 4) / 5 < (enemyHealth[enemy2] + 4) / 5) {
                    return true;
                }
                if ((enemyHealth[enemy1] + 4) / 5 > (enemyHealth[enemy2] + 4) / 5) {
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

//    std::cerr << "tick: " << playerView->currentTick << ", shoot: " << enemyId << std::endl;
    actions[myId] = AttackAction(enemyId);
}

std::vector<Vec2Int> MyStrategy::getBuildingEdges(const Vec2Int& position,
                                                  int size,
                                                  const std::unordered_set<EntityType>& entityTypes) {
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
                if (checkWorldBounds(edge.x, edge.y) && (world(edge).isEmpty() || world(edge).inEntityTypes(entityTypes))) {
                    buildPositions.emplace_back(edge);
                }
        }
    }
    return buildPositions;
}

std::vector<Vec2Int> MyStrategy::getBuildingEdges(int buildingId) {
    const auto& building = playerView->entitiesById.at(buildingId);
    int entitySize = playerView->entityProperties.at(building.entityType).size;
    return getBuildingEdges(building.position, entitySize, {});
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
            }
        }
        if (isRepairer) {
            edges.erase(std::find(edges.begin(), edges.end(), topPosition));
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                return world(v1).myBuildingsBfs > world(v2).myBuildingsBfs;
            });
            edges.push_back(topPosition);
            std::reverse(edges.begin(), edges.end());
            for (const auto& edge : edges) {
                addMove(builderUnit.id, edge, world(edge).myBuildingsBfs, 25);
            }
        }
    }

    std::vector<int> busyBuildersVec{busyBuilders.begin(), busyBuilders.end()};
    for (const auto& brokenBuilding : brokenBuildings) {
        const auto& building = playerView->entitiesById.at(brokenBuilding);
        if (building.active) {
            continue;
        }
        int maxBuildersCount = building.entityType == RANGED_BASE ? 12 : 5;
        int maxBuildersDist = building.entityType == RANGED_BASE ? 15 : 6;
        std::vector<Vec2Int> buildingEdges = getBuildingEdges(brokenBuilding);

        int count = repairersCounts[brokenBuilding];
        while (true) {
            std::vector<int> builderIds;
            const auto& bfsResult = bfs(buildingEdges, OBSTACLE_TYPES, busyBuildersVec, builderIds);
            if (builderIds.empty()) {
                break;
            }
            const auto& builderUnit = playerView->entitiesById.at(builderIds[0]);
            if (++count > maxBuildersCount || bfsResult[builderUnit.position.x][builderUnit.position.y].score > maxBuildersDist) {
                break;
            }
            const auto& positionToRemove = bfsResult[builderUnit.position.x][builderUnit.position.y].position;
            buildingEdges.erase(std::find(buildingEdges.begin(), buildingEdges.end(), positionToRemove));
            std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                return bfsResult[v1.x][v1.y].score < bfsResult[v2.x][v2.y].score;
            });
            for (const auto& edge : edges) {
                addMove(builderIds[0], edge, bfsResult[edge.x][edge.y].score, 20);
            }
//            std::cerr << "tick: " << playerView->currentTick << ", REPAIRER pos: " << builderUnit.position
//                      << ", target: " << positionToRemove << std::endl;
            busyBuilders.insert(builderIds[0]);
            busyBuildersVec.push_back(builderIds[0]);
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
    int maxHousesCount = isFinal ? 11 : 6;
    if (potentialBuilders.empty()) {
        int maxInactiveHousesCount = playerView->getMyEntities(RANGED_BASE).empty() ? 1 : 3;
        int housesCount = playerView->getMyEntities(HOUSE).size();
        if (playerView->getFood() < 10
                && ((myPlayer.resource >= 50 && playerView->getInactiveHousesCount() < 1)
                || (myPlayer.resource >= 200 && playerView->getInactiveHousesCount() < 2))
                && (!playerView->getMyEntities(RANGED_BASE).empty() || housesCount < maxHousesCount)) {
            std::vector<Vec2Int> housePositions;
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
        if (buildType == HOUSE) {
            std::sort(potentialBuilders.begin(), potentialBuilders.end(), [&] (const PotentialBuilder& builder1, const PotentialBuilder& builder2) {
                return builder1.score < builder2.score;
            });
        } else if (buildType == RANGED_BASE) {
            std::sort(potentialBuilders.begin(), potentialBuilders.end(), [&] (const PotentialBuilder& builder1, const PotentialBuilder& builder2) {
                return builder1.score * 2 + dist({27, 27}, builder1.position) < builder2.score * 2 + dist({27, 27}, builder2.position);
            });
        }

        PotentialBuilder bestHouseBuilder;
        int counter = 0;
        bool isBestHouseBuilderChoosen = false;
        int buildingSquare = (2 * buildingSize + 1) * (2 * buildingSize + 1);
        for (const auto& potentialBuilder : potentialBuilders) {
            std::unordered_set<Vec2Int> blockedCells;
            for (int i = potentialBuilder.position.x - buildingSize; i <= potentialBuilder.position.x + buildingSize; ++i) {
                for (int j = potentialBuilder.position.y - buildingSize; j <= potentialBuilder.position.y + buildingSize; ++j) {
                    blockedCells.insert({i, j});
                }
            }
            const Vec2Int& startPos = playerView->entitiesById.at(potentialBuilder.unitId).position;
            int buildingEmptiness = bfsForValidateBuildings({startPos}, OBSTACLE_TYPES, blockedCells);
            int withoutBuildingEmptiness = bfsForValidateBuildings({startPos}, OBSTACLE_TYPES);
//            if (counter < 3) {
//                std::cerr << "tick: " << playerView->currentTick << ", pos: " << potentialBuilder.position
//                          << ", val: " << withoutBuildingEmptiness - buildingEmptiness - buildingSquare << std::endl;
//            }
            bool nearBase = potentialBuilder.position.x <= 11 && potentialBuilder.position.y <= 11
                    && potentialBuilder.position.x >= 3 && potentialBuilder.position.y >= 3;
            if (nearBase || withoutBuildingEmptiness - buildingEmptiness - buildingSquare < 10) {
                bestHouseBuilder = potentialBuilder;
                isBestHouseBuilderChoosen = true;
                break;
            }
            if (++counter > 10) {
                break;
            }
        }
        if (!isBestHouseBuilderChoosen) {
//            std::cerr << "tick: " << playerView->currentTick << ", TAKE FIIIIRST :(" << std::endl;
            bestHouseBuilder = potentialBuilders[0];
        }

        std::vector<Vec2Int> buildingEdges = getBuildingEdges(
                {bestHouseBuilder.position.x - buildingSize, bestHouseBuilder.position.y - buildingSize},
                buildingSize * 2 + 1,
                {MELEE_UNIT, RANGED_UNIT, BUILDER_UNIT}
        );
        int count = 0;
        if (std::find(buildingEdges.begin(), buildingEdges.end(), playerView->entitiesById.at(bestHouseBuilder.unitId).position) != buildingEdges.end()) {
            busyBuilders.insert(bestHouseBuilder.unitId);
            builderAttackActions[bestHouseBuilder.unitId] = BuildAction(buildType,
                                                                        {bestHouseBuilder.position.x - buildingSize,
                                                                         bestHouseBuilder.position.y - buildingSize});
            ++count;
        }
        int maxBuildersCount = buildType == RANGED_BASE ? 12 : 5;
        int maxBuildersDist = buildType == RANGED_BASE ? 12 : 6;
        std::vector<int> busyBuildersVec{busyBuilders.begin(), busyBuilders.end()};

        while (true) {
            std::vector<int> builderIds;
            const auto& bfsResult = bfs(buildingEdges, OBSTACLE_TYPES, busyBuildersVec, builderIds);
            if (builderIds.empty()) {
                break;
            }
            const auto& builderUnit = playerView->entitiesById.at(builderIds[0]);
            if (++count > maxBuildersCount || bfsResult[builderUnit.position.x][builderUnit.position.y].score > maxBuildersDist) {
                break;
            }
            const auto& positionToRemove = bfsResult[builderUnit.position.x][builderUnit.position.y].position;
            buildingEdges.erase(std::find(buildingEdges.begin(), buildingEdges.end(), positionToRemove));
            std::vector<Vec2Int> edges = getEdges(builderUnit.position, true);
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                return bfsResult[v1.x][v1.y].score < bfsResult[v2.x][v2.y].score;
            });
            for (const auto& edge : edges) {
                addMove(builderIds[0], edge, bfsResult[edge.x][edge.y].score, 20);
            }
//            std::cerr << "tick: " << playerView->currentTick << ", (not built) REPAIRER pos: " << builderUnit.position
//                      << ", target: " << positionToRemove << std::endl;
            busyBuilders.insert(builderIds[0]);
            busyBuildersVec.push_back(builderIds[0]);
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
                    addMove(builderUnit.id, edge, enemyMap[edge.x][edge.y], 15);
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
            std::sort(edges.begin(), edges.end(), [&](const Vec2Int& v1, const Vec2Int& v2) {
                return world(v1).resourcesBfs > world(v2).resourcesBfs;
            });
            edges.push_back(builderUnit.position);
            std::reverse(edges.begin(), edges.end());
            for (const auto& edge : edges) {
                addMove(builderUnit.id, edge, world(edge).resourcesBfs, 25);
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
    buildersPositions.emplace_back(5, 5);
    buildersPositions.emplace_back(5, 9);
    buildersPositions.emplace_back(9, 5);
    buildersPositions.emplace_back(9, 9);

    std::unordered_set<Vec2Int> busyCells;
    for (const auto& builder : busyBuilders) {
        busyCells.insert(playerView->entitiesById.at(builder).position);
    }

    const auto& bfsResult = bfsBuilderResources(buildersPositions, obstacleTypes, busyCells);
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
            buildersToTarget[builder] = {70, 70};
            continue;
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
//        std::cerr << "Tick: " << playerView->currentTick << ", MOVING_TO_FARM pos: " << builderPtr->position
//                  << ", target: " << target << ", move to: " << edges[0] << std::endl;
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
