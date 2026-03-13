#pragma once

#include <Geode/Prelude.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/VMTHookManager.hpp>
#include <manager/AttemptStorage.hpp>

using namespace geode::prelude;

class $modify(LPPlayLayer, PlayLayer) {
    struct Fields {
        bool activeLastTick = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects);
    void resetLevel();
    void destroyPlayer(PlayerObject* player, GameObject* object);
    void customUpdate(float);
};