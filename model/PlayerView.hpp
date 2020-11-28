#ifndef _MODEL_PLAYER_VIEW_HPP_
#define _MODEL_PLAYER_VIEW_HPP_

#include "../Stream.hpp"
#include "AttackProperties.hpp"
#include "BuildProperties.hpp"
#include "Entity.hpp"
#include "EntityProperties.hpp"
#include "EntityType.hpp"
#include "Player.hpp"
#include "RepairProperties.hpp"
#include "Vec2Int.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using Entities = std::unordered_map<EntityType, std::vector<Entity>>;

class PlayerView {
public:
    int myId;
    int mapSize;
    bool fogOfWar;
    std::unordered_map<EntityType, EntityProperties> entityProperties;
    int maxTickCount;
    int maxPathfindNodes;
    int currentTick;
    std::vector<Player> players;
    std::vector<Entity> entities;
    PlayerView();
    PlayerView(int myId, int mapSize, bool fogOfWar, std::unordered_map<EntityType, EntityProperties> entityProperties, int maxTickCount, int maxPathfindNodes, int currentTick, std::vector<Player> players, std::vector<Entity> entities);

    std::unordered_map<int, Player> playersById;
    std::unordered_map<int, Entities> entitiesByPlayerId;
    std::unordered_map<int, Entity> entitiesById;

    int getFood() const;

    const std::vector<Entity>& GetMyEntities(EntityType entityType) const;

    static PlayerView readFrom(InputStream& stream);
    void writeTo(OutputStream& stream) const;
};

#endif
