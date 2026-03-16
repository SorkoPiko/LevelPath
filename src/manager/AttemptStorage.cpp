#include "AttemptStorage.hpp"

#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/file.hpp>

#include "serialise/Float16.hpp"

AttemptStorage::AttemptStorage() {
    (void) file::createDirectoryAll(dirs::getSaveDir() / "attempts");
}

void AttemptStorage::start(const LevelIdentifier levelID) {
    if (active) {
        if (activeID == levelID) return;
        commit();
    }
    active = true;
    activeID = levelID;

    p1Ticks.clear();
    p2Ticks.clear();
}

void AttemptStorage::apply(PlayerObject* player, const bool secondary) {
    if (!active) return;

    std::vector<AttemptTick>& ticks = secondary ? p2Ticks : p1Ticks;

    ticks.emplace_back(AttemptTick{
        player->getPositionX(), player->getPositionY(),
        player->getRotation(),
        fromPlayer(player), player->m_isUpsideDown, player->m_vehicleSize < 1.0f
    });
}

void AttemptStorage::commit() {
    if (!active) return;
    active = false;
    if (p1Ticks.empty() && p2Ticks.empty()) return;

    saveQueue.scheduleSave(activeID, std::move(p1Ticks), std::move(p2Ticks));
}