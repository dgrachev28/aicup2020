#include "AttackAction.hpp"

#include <utility>

AttackAction::AttackAction() { }
AttackAction::AttackAction(std::optional<int> target, std::optional<AutoAttack> autoAttack) : target(target), autoAttack(std::move(autoAttack)) { }
AttackAction::AttackAction(std::optional<int> target) : target(target) { }
AttackAction::AttackAction(const AutoAttack& autoAttack) : autoAttack(autoAttack) { }

AttackAction AttackAction::readFrom(InputStream& stream) {
    AttackAction result;
    if (stream.readBool()) {
        result.target = stream.readInt();
    } else {
        result.target = std::nullopt;
    }
    if (stream.readBool()) {
        result.autoAttack = AutoAttack::readFrom(stream);
    } else {
        result.autoAttack = std::nullopt;
    }
    return result;
}
void AttackAction::writeTo(OutputStream& stream) const {
    if (target) {
        stream.write(true);
        stream.write((*target));
    } else {
        stream.write(false);
    }
    if (autoAttack) {
        stream.write(true);
        (*autoAttack).writeTo(stream);
    } else {
        stream.write(false);
    }
}
