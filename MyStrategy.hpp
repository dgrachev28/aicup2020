#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include <set>
#include "DebugInterface.hpp"
#include "model/Model.hpp"

using Actions = std::unordered_map<int, EntityAction>;

struct DistId {
    int distance;
    int entityId;

    bool operator<(const DistId& distId) const {
        return distance < distId.distance;
    }
};

class MyStrategy {
public:
    int houseBuilderId = -1;

    int world[80][80];

    std::unordered_map<int, std::set<DistId>> myToMyMapping;
    std::unordered_map<int, std::set<DistId>> myToEnemyMapping;
    std::unordered_map<int, std::set<DistId>> enemyToMyMapping;
    std::unordered_map<int, std::set<DistId>> enemyToEnemyMapping;

    MyStrategy();
    Action getAction(const PlayerView& playerView, DebugInterface* debugInterface);
    void debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface);

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

};

#endif