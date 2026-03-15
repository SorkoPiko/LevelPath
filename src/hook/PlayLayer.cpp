#include "PlayLayer.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>

bool LPPlayLayer::init(GJGameLevel* level, const bool useReplay, const bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    (void)VMTHookManager::get().addHook<ResolveC<LPPlayLayer>::func(&LPPlayLayer::processQueuedButtons)>(this, "PlayLayer::processQueuedButtons");
    schedule(schedule_selector(LPPlayLayer::customUpdate), 0.0f, kCCRepeatForever, 0.0f);

    return true;
}

void LPPlayLayer::resetLevel() {
    PlayLayer::resetLevel();

    AttemptStorage& storage = AttemptStorage::get();
    storage.commit();

    storage.start(fromLevel(m_level));
}

void LPPlayLayer::destroyPlayer(PlayerObject* player, GameObject* object) {
    PlayLayer::destroyPlayer(player, object);
    if (object == m_anticheatSpike) return;

    AttemptStorage::get().commit();
}

void LPPlayLayer::customUpdate(float) {
    if (!m_fields->activeLastTick) return;
    m_fields->activeLastTick = false;

    AttemptStorage& storage = AttemptStorage::get();
    if (m_player1) storage.apply(m_player1, false);
    if (m_player2) storage.apply(m_player2, true);
}