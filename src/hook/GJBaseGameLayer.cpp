#include <Geode/Prelude.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <manager/AttemptStorage.hpp>

#include "PlayLayer.hpp"

class $modify(GJBaseGameLayer) {
    void processQueuedButtons(const float dt, const bool clearInputQueue) {
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
        const auto layer = reinterpret_cast<LPPlayLayer*>(this);
        layer->m_fields->activeLastTick = true;
    }
};