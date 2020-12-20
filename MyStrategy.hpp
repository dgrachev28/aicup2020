#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include <set>
#include <map>
#include <unordered_set>
#include "DebugInterface.hpp"
#include "model/Model.hpp"
#include <ostream>
#include <array>

using Actions = std::unordered_map<int, EntityAction>;
using EntityPtr = const Entity*;

struct Score {
    float myBuildingScore;
    float myBuilderScore;
    float myPowerScore;
    float enemyPowerScore;
    float powerScore = 0.0f;
    float score = 0.0f;

    void calcScore();
    void calcScore2();
    void calcAttackScore();

    bool operator<(const Score& other) const {
        return score < other.score;
    }
    friend std::ostream& operator<<(std::ostream& out, const Score& score);
};

struct DistId {
    int distance;
    int entityId;

    bool operator<(const DistId& distId) const {
        if (distance < distId.distance) {
            return true;
        }
        if (distance > distId.distance) {
            return false;
        }
        return entityId < distId.entityId;
    }
};

struct DistPositionId {
    int distance;
    Vec2Int position;
    int entityId;
};

struct PotentialCell {
    Score score;
    int x;
    int y;
    int count;

    bool operator<(const PotentialCell& other) const {
        return score.score < other.score.score;
    }

    friend std::ostream& operator<<(std::ostream& out, const PotentialCell& potentialCell);
};

struct MoveStep {
    int unitId;
    Vec2Int target;
    int score;
    int index = 0;
};

//struct MovePlan {
//    int priority;
//    int unitId;
//    std::vector<MoveStep> moves;
//};

struct Cell {
    int x;
    int y;

    bool turretDanger = false;
    const Entity* entity = nullptr;

    bool isEmpty() const {
        return entity == nullptr;
    }

    int getEntityId() const {
        if (isEmpty()) {
            return -1;
        }
        return entity->id;
    }

    EntityType getEntityType() const {
        return entity->entityType;
    }

    bool eqEntityType(EntityType entityType) const {
        return entity && entity->entityType == entityType;
    }

    bool inEntityTypes(const std::unordered_set<EntityType>& entityTypes) const {
        return entity && entityTypes.contains(entity->entityType);
    }

    bool eqPlayerId(int playerId) const {
        return entity && entity->playerId == playerId;
    }

    int getPlayerId() const {
        return entity->playerId;
    }
};

using World = std::array<std::array<Cell, 80>, 80>;

enum class BuilderState {
    FARM,
    MOVE_TO_FARM,
    REPAIR,
    BUILD_HOUSE,
    MOVE_TO_BUILD_HOUSE
};

class BuilderMeta {
public:
    BuilderState state;
    std::optional<Vec2Int> target;

    BuilderMeta() {}
    BuilderMeta(BuilderState state) : state(state), target(std::nullopt) {}
    BuilderMeta(BuilderState state, std::optional<Vec2Int> target) : state(state), target(target) {}
};

struct Scout {
    Vec2Int target;
    int unitId;
};

enum class MicroState {
    ATTACK, STAY, RUN_AWAY
};

struct CollisionPriority {
    int score;
    std::vector<EntityPtr> units;
    Vec2Int position;

    bool operator<(const CollisionPriority& other) const {
        if (position == other.position) {
            return false;
        }
        if (units.size() < other.units.size()) {
            return true;
        }
        if (other.units.size() < units.size()) {
            return false;
        }
        if (score < other.score) {
            return true;
        }
        if (other.score < score) {
            return false;
        }
        return position < other.position;
    }

    friend std::ostream& operator<<(std::ostream& out, const CollisionPriority& score);
};

struct PotentialBuilder {
    int unitId;
    float score;
    Vec2Int position;
};

class MyStrategy {
public:
    const PlayerView* playerView;

    std::unordered_map<int, BuilderMeta> builderMeta;
    int edgeHousesShiftX;
    int edgeHousesShiftY;
//    std::unordered_map<Vec2Int, std::vector<Vec2Int>> edgesMap;
    std::array<std::array<std::vector<Vec2Int>, 80>, 80> edgesMap;

    std::unordered_map<int, std::vector<DistId>> myToMyMapping;
    std::unordered_map<int, std::vector<DistId>> myToEnemyMapping;
    std::unordered_map<int, std::vector<DistId>> enemyToMyMapping;
    std::unordered_map<int, std::vector<DistId>> enemyToEnemyMapping;

//    std::unordered_map<int, std::vector<DistId>> entitiesMapping;

    std::array<std::array<int, 80>, 80> enemyMap;
    std::array<std::array<int, 80>, 80> myMap;

    std::map<int, std::unordered_set<int>> movePriorityToUnitIds;
    std::unordered_map<int, std::vector<MoveStep>> unitMoveSteps;
    Actions builderAttackActions;

    std::vector<Vec2Int> farmTargets_;

    std::unordered_map<int, Vec2Int> lastTargetPositions;

    std::unordered_set<Vec2Int> freeScoutSpots;
    std::unordered_map<int, Vec2Int> scouts;

    bool isFinal;

    MyStrategy();
    Action getAction(const PlayerView& playerView, DebugInterface* debugInterface);
    void debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface);

    std::array<std::array<int, 80>, 80>
    bfs(const std::vector<Vec2Int>& startCells,
        const std::unordered_set<EntityType>& obstacleTypes,
        const std::vector<int>& obstacleUnitIds);

    std::array<std::array<int, 80>, 80>
    bfs(const std::vector<Vec2Int>& startCells,
        const std::unordered_set<EntityType>& obstacleTypes,
        const std::vector<int>& obstacleUnitIds,
        std::vector<int>& closestUnits);

    std::array<std::array<int, 80>, 80> bfs(const std::vector<Vec2Int>& startCells);

    std::vector<Vec2Int> bfsBuilderResources(const std::vector<Vec2Int>& startCells,
        const std::unordered_set<EntityType>& obstacleTypes,
        const std::unordered_set<int>& obstacleUnitIds);
    std::array<std::array<int, 80>, 80> dijkstra(const std::vector<Vec2Int>& startCells, bool isWeighted);

private:
    World world_;
    Cell& world(int x, int y);
    Cell& world(const Vec2Int& v);
    bool checkWorldBounds(int x, int y);

    void stepInit(const PlayerView& playerView);

    void getBuildUnitActions(const PlayerView& playerView, Actions& actions);
    void setBuilderUnitsActions(Actions& actions);

    std::unordered_map<int, std::vector<DistId>> calculateDistances(
            const PlayerView &playerView,
            int keyPlayerId,
            int valuePlayerId
    );

    // Building buildings :)
    bool checkBuilderUnit(int x, int y, const std::unordered_set<int>& busyBuilders);

    int calcBlockingFarmScore(int x, int y, int size);
    PotentialBuilder calcBuildingPlaceScore(int x, int y, int size, const std::unordered_set<int>& busyBuilders);
    bool isEmptyForHouse(int x, int y, int size, const std::unordered_set<int>& busyBuilders);

    // End of Building buildings

    EntityAction createBuildUnitAction(const Entity& base, EntityType unitType);
    EntityAction createBuildUnitAction2(const Entity& base, EntityType unitType, const Vec2Int& target);


    // Ranged units actions
    void getRangedUnitAction(const PlayerView& playerView, Actions& actions);

    float potentialField[80][80];
    std::vector<PotentialCell> topPotentials;
    std::vector<PotentialCell> topAttackPotentials;

    void findTargetEnemies(const PlayerView& playerView);
    void createPotentialField(const PlayerView& playerView);
    void fillPotential(int x, int y, float score);
    Vec2Int getWarriorTargetPosition(const Entity &unit);
    void moveBattleUnits(Actions& actions);

    void battleDfs(int unitId, std::unordered_set<int>& groupedUnits, std::unordered_map<int, int>& group);

    // End of Ranged units actions

    void addMove(int unitId, const Vec2Int& target, int score, int priority);
    void handleMoves(Actions& actions);

    // Shoot
    void handleAttackActions(Actions& actions);
    void handleBuilderAttackActions(Actions& actions);

    void resolveSimpleShoots(
            std::unordered_map<int, int>& enemyHealth,
            std::unordered_map<int, std::unordered_set<int>>& myToEnemy,
            std::unordered_map<int, std::unordered_set<int>>& enemyToMy,
            Actions& actions
    );

    void shoot(
            int myId,
            int enemyId,
            std::unordered_map<int, int>& enemyHealth,
            std::unordered_map<int, std::unordered_set<int>>& myToEnemy,
            std::unordered_map<int, std::unordered_set<int>>& enemyToMy,
            Actions& actions
    );
    // End of Shoot

    // Builders find resources
    void setRepairBuilders(std::unordered_set<int>& busyBuilders, Actions& actions);
    void setHouseBuilders(std::unordered_set<int>& busyBuilders, Actions& actions);
    void setRunningFromEnemyBuilders(std::unordered_set<int>& busyBuilders, Actions& actions);
    void setAttackBuilders(std::unordered_set<int>& busyBuilders, Actions& actions);
    void setFarmers(std::unordered_set<int>& busyBuilders, Actions& actions);
    void setMovingToFarm(std::unordered_set<int>& busyBuilders, Actions& actions);

    std::vector<Vec2Int> getBuildingEdges(const Vec2Int& position, int size, bool areUnitsEmpty);
    std::vector<Vec2Int> getBuildingEdges(int buildingId);
};

#endif