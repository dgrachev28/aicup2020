#include "MyStrategy.hpp"
#include <exception>
#include <iostream>


int dist(const Entity& e1, const Entity& e2) {
    return abs(e1.position.x - e2.position.x) + abs(e1.position.y - e2.position.y);
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
    getBuildBaseActions(playerView, actions);
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

    const Entity& builderBase = playerView.GetMyEntities(BUILDER_BASE)[0];
    const Entity& rangedBase = playerView.GetMyEntities(RANGED_BASE)[0];
    if (buildersCount < 15) {
        actions[builderBase.id] = BuildAction(BUILDER_UNIT, {builderBase.position.x, builderBase.position.y - 1});
    } else if (buildersCount < 80) {
        if (rangedCount < buildersCount - 12) {
            actions[rangedBase.id] = BuildAction(RANGED_UNIT, {rangedBase.position.x, rangedBase.position.y + 5});
        } else {
            actions[builderBase.id] = BuildAction(BUILDER_UNIT, {builderBase.position.x, builderBase.position.y + 5});
        }
    } else {
        actions[rangedBase.id] = BuildAction(RANGED_UNIT, {rangedBase.position.x, rangedBase.position.y + 5});
    }
}

void MyStrategy::getBuildBaseActions(const PlayerView& playerView, Actions& actions) {
    if (houseBuilderId == -1) {
        if (playerView.getFood() == 0) {
            houseBuilderId = playerView.GetMyEntities(BUILDER_UNIT)[0].id;
        } else {
            return;
        }
    }

    const auto& houses = playerView.GetMyEntities(HOUSE);
    int housesCount = houses.size();

    int notActiveHouseId = -1;
    for (const Entity& house : houses) {
        if (!house.active) {
            notActiveHouseId = house.id;
        }
    }
    if (notActiveHouseId != -1) {
        actions[houseBuilderId] = RepairAction(notActiveHouseId);
    } else {
        Vec2Int pos;
        for (const Entity& builder : playerView.GetMyEntities(BUILDER_UNIT)) {
            if (builder.id == houseBuilderId) {
                pos = builder.position;
            }
        }
        if (housesCount < 26) {
            Vec2Int targetPos{0 + housesCount * 3, 3};
            if (pos == targetPos) {
                actions[houseBuilderId] = BuildAction(HOUSE, {0 + housesCount * 3, 0});
            } else {
                actions[houseBuilderId] = MoveAction(targetPos, false, false);
            }
        }
    }
}

void MyStrategy::getFarmerActions(const PlayerView& playerView, Actions& actions) {
    const auto& builders = playerView.GetMyEntities(BUILDER_UNIT);
    for (const Entity& builder : builders) {
        if (builder.id != houseBuilderId) {
            actions[builder.id] = AttackAction({1000, {RESOURCE}});
        }
    }
}

void MyStrategy::getWarriorActions(const PlayerView& playerView, Actions& actions) {
    const auto& units = playerView.GetMyEntities(RANGED_UNIT);
    int rangedCount = units.size();
    for (const Entity& unit : units) {
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
