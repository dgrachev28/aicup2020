#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include <set>
#include <map>
#include <unordered_set>
#include "DebugInterface.hpp"
#include "model/Model.hpp"
#include <ostream>

using Actions = std::unordered_map<int, EntityAction>;

struct Score {
    float myBuildingScore;
    float myBuilderScore;
    float myPowerScore;
    float enemyPowerScore;
    float powerScore = 0.0f;
    float score = 0.0f;

    void calcScore();

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

struct PotentialCell {
    Score score;
    int x;
    int y;

    bool operator<(const PotentialCell& other) const {
        return score.score < other.score.score;
    }

    friend std::ostream& operator<<(std::ostream& out, const PotentialCell& potentialCell);
};

struct MoveStep {
    int unitId;
    int score;
    Vec2Int target;
};

//struct MovePlan {
//    int priority;
//    int unitId;
//    std::vector<MoveStep> moves;
//};

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

class MyStrategy {
public:
    const PlayerView* playerView;

    int houseBuilderId = -1;

    std::unordered_map<int, BuilderMeta> builderMeta;
    int edgeHousesShiftX;
    int edgeHousesShiftY;

    int world[80][80];

    std::unordered_map<int, std::set<DistId>> myToMyMapping;
    std::unordered_map<int, std::set<DistId>> myToEnemyMapping;
    std::unordered_map<int, std::set<DistId>> enemyToMyMapping;
    std::unordered_map<int, std::set<DistId>> enemyToEnemyMapping;

//    std::unordered_map<int, std::set<DistId>> entitiesMapping;

    std::map<int, int> movePriorityToUnitId;
    std::unordered_map<int, std::vector<MoveStep>> unitMoveSteps;

    std::unordered_map<int, Vec2Int> lastTargetPositions;

    MyStrategy();
    Action getAction(const PlayerView& playerView, DebugInterface* debugInterface);
    void debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface);

    std::vector<std::vector<int>> dijkstra(const std::vector<Vec2Int>& startCells, bool isWeighted);

private:
    void getBuildUnitActions(const PlayerView& playerView, Actions& actions);
    void getBuildBaseActions(const PlayerView& playerView, Actions& actions);
    void getFarmerActions(const PlayerView& playerView, Actions& actions);
    void getWarriorActions(const PlayerView& playerView, Actions& actions);

    std::unordered_map<int, std::set<DistId>> calculateDistances(
            const PlayerView &playerView,
            int keyPlayerId,
            int valuePlayerId
    );

    bool checkBuilderUnit(const PlayerView& playerView, int x, int y);

    int isEmptyForHouse(const PlayerView& playerView, int x, int y, int size);

    BuildAction createBuildUnitAction(const Entity& base, EntityType unitType, bool isAggresive);


    // Ranged units actions
    void getRangedUnitAction(const PlayerView& playerView, Actions& actions);

    float potentialField[80][80];
    std::vector<PotentialCell> topPotentials;

    void findTargetEnemies(const PlayerView& playerView);
    void createPotentialField(const PlayerView& playerView);
    void fillPotential(int x, int y, float score);
    Vec2Int getWarriorTargetPosition(const Entity &unit);
    void moveBattleUnits(Actions& actions);

    void battleDfs(int unitId, std::unordered_set<int>& groupedUnits, std::unordered_map<int, int>& group);

    // End of Ranged units actions
};

#endif