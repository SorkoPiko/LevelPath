#pragma once

#include <vector>
#include <Geode/Prelude.hpp>
#include <Geode/binding/PlayerObject.hpp>

#include "SaveQueue.hpp"
#include "model/AttemptTick.hpp"

using namespace geode::prelude;

class AttemptStorage {
    bool active = false;
    int activeID = -1;
    std::vector<AttemptTick> p1Ticks;
    std::vector<AttemptTick> p2Ticks;

    SaveQueue saveQueue;

    AttemptStorage();

public:
    static AttemptStorage& get() {
        static AttemptStorage instance;
        return instance;
    }

    void start(int levelID);
    void apply(PlayerObject* player, bool secondary);
    void commit();

    SaveQueue& getSaveQueue() {
        return saveQueue;
    }

    bool isActive() const {
        return active;
    }
};