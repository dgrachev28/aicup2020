#include "EntityAction.hpp"

EntityAction::EntityAction() { }
EntityAction::EntityAction(std::optional<MoveAction> moveAction, std::optional<BuildAction> buildAction, std::optional<AttackAction> attackAction, std::optional<RepairAction> repairAction) : moveAction(moveAction), buildAction(buildAction), attackAction(attackAction), repairAction(repairAction) { }
EntityAction EntityAction::readFrom(InputStream& stream) {
    EntityAction result;
    if (stream.readBool()) {
        result.moveAction = MoveAction::readFrom(stream);
    } else {
        result.moveAction = std::optional<MoveAction>();
    }
    if (stream.readBool()) {
        result.buildAction = BuildAction::readFrom(stream);
    } else {
        result.buildAction = std::optional<BuildAction>();
    }
    if (stream.readBool()) {
        result.attackAction = AttackAction::readFrom(stream);
    } else {
        result.attackAction = std::optional<AttackAction>();
    }
    if (stream.readBool()) {
        result.repairAction = RepairAction::readFrom(stream);
    } else {
        result.repairAction = std::optional<RepairAction>();
    }
    return result;
}
void EntityAction::writeTo(OutputStream& stream) const {
    if (moveAction) {
        stream.write(true);
        (*moveAction).writeTo(stream);
    } else {
        stream.write(false);
    }
    if (buildAction) {
        stream.write(true);
        (*buildAction).writeTo(stream);
    } else {
        stream.write(false);
    }
    if (attackAction) {
        stream.write(true);
        (*attackAction).writeTo(stream);
    } else {
        stream.write(false);
    }
    if (repairAction) {
        stream.write(true);
        (*repairAction).writeTo(stream);
    } else {
        stream.write(false);
    }
}

EntityAction::EntityAction(const MoveAction &action) : moveAction(action) {}
EntityAction::EntityAction(const BuildAction &action) : buildAction(action) {}
EntityAction::EntityAction(const AttackAction &action) : attackAction(action) {}
EntityAction::EntityAction(const RepairAction &action) : repairAction(action) {}
