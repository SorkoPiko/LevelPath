#include "AttemptStorage.hpp"

#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/file.hpp>

#include "serialise/Float16.hpp"

AttemptStorage::AttemptStorage() {
    (void) file::createDirectoryAll(dirs::getSaveDir() / "attempts");
}

void AttemptStorage::start(const LevelIdentifier levelID, const int recordingRate, const bool fromStart) {
    if (active) {
        if (activeID == levelID) return;
        commit();
    }
    active = true;
    activeID = levelID;

    currentAttempt.emplace();
    currentAttempt->recordingRate = std::min(recordingRate, 255);
    currentAttempt->fromStart = fromStart;
}

void AttemptStorage::apply(PlayerObject* player, const bool secondary) {
    if (!active) return;

    std::vector<AttemptTick>& ticks = secondary ? currentAttempt->p2Ticks : currentAttempt->p1Ticks;

    ticks.emplace_back(AttemptTick{
        player->getPositionX(), player->getPositionY(),
        player->getRotation(),
        fromPlayer(player), player->m_mainLayer->getScaleY() < 0.0f, player->m_vehicleSize < 1.0f
    });
}

void AttemptStorage::commit() {
    if (!active) return;
    active = false;
    if (currentAttempt->p1Ticks.empty() && currentAttempt->p2Ticks.empty()) return;

    saveQueue.scheduleAppend(activeID, std::move(*currentAttempt));
    currentAttempt.reset();
}