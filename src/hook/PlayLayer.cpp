#include "PlayLayer.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>

bool LPPlayLayer::init(GJGameLevel* level, const bool useReplay, const bool dontCreateObjects) {
    m_fields->recordingRate = Mod::get()->getSettingValue<int>("recording-rate");
    if (!Mod::get()->getSettingValue<bool>("record-paths")) {
        m_fields->recordingRate = 0;
    }

    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    if (m_fields->recordingRate <= 0) return true;

    (void)VMTHookManager::get().addHook<ResolveC<LPPlayLayer>::func(&LPPlayLayer::processQueuedButtons)>(this, "PlayLayer::processQueuedButtons");
    schedule(schedule_selector(LPPlayLayer::customUpdate), 1.0f / m_fields->recordingRate, kCCRepeatForever, 0.0f);

    return true;
}

void LPPlayLayer::resetLevel() {
    PlayLayer::resetLevel();
    if (m_fields->recordingRate <= 0) return;

    AttemptStorage& storage = AttemptStorage::get();
    storage.commit();

    log::debug("resetting level, pos {}", m_player1->getPositionX() == 0.0f);
    storage.start(fromLevel(m_level), m_fields->recordingRate, m_player1->getPositionX() == 0.0f);
}

void LPPlayLayer::destroyPlayer(PlayerObject* player, GameObject* object) {
    PlayLayer::destroyPlayer(player, object);
    if (m_fields->recordingRate <= 0) return;

    if (object == m_anticheatSpike) return;

    AttemptStorage& storage = AttemptStorage::get();
    if (m_player1) storage.apply(m_player1, false);
    if (m_player2 && m_gameState.m_isDualMode) storage.apply(m_player2, true);
    storage.commit();
}

void LPPlayLayer::levelComplete() {
    PlayLayer::levelComplete();
    if (m_fields->recordingRate <= 0) return;

    AttemptStorage& storage = AttemptStorage::get();
    if (m_player1) storage.apply(m_player1, false);
    if (m_player2 && m_gameState.m_isDualMode) storage.apply(m_player2, true);
    storage.commit();
}

void LPPlayLayer::onQuit() {
    AttemptStorage& storage = AttemptStorage::get();
    storage.commit();

    PlayLayer::onQuit();
}

void LPPlayLayer::customUpdate(float) {
    if (!m_fields->activeLastTick) return;
    m_fields->activeLastTick = false;

    AttemptStorage& storage = AttemptStorage::get();
    if (m_player1) storage.apply(m_player1, false);
    if (m_player2 && m_gameState.m_isDualMode) storage.apply(m_player2, true);
}