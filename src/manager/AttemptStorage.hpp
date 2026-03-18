#pragma once

#include <Geode/Prelude.hpp>
#include <Geode/binding/PlayerObject.hpp>

#include "SaveQueue.hpp"

using namespace geode::prelude;

class AttemptStorage {
    bool active = false;
    LevelIdentifier activeID = {-1, GJLevelType::Main};
    std::optional<PathAttempt> currentAttempt;

    SaveQueue saveQueue;

    AttemptStorage();

public:
    static AttemptStorage& get() {
        static AttemptStorage instance;
        return instance;
    }

    void start(LevelIdentifier levelID, int recordingRate);
    void apply(PlayerObject* player, bool secondary);
    void commit();

    SaveQueue& getSaveQueue() {
        return saveQueue;
    }

    bool isActive() const {
        return active;
    }
};