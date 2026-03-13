#include <Geode/Prelude.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <manager/AttemptStorage.hpp>

using namespace geode::prelude;

class $modify(LPPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, const bool useReplay, const bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        schedule(schedule_selector(LPPlayLayer::customUpdate), 0.0f, kCCRepeatForever, 0.0f);

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        AttemptStorage& storage = AttemptStorage::get();
        storage.commit();
        storage.start(m_level->m_levelID.value());
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (object == m_anticheatSpike) return;

        AttemptStorage::get().commit();
    }

    void customUpdate(float) {
        if (m_isPaused) return;

        AttemptStorage& storage = AttemptStorage::get();
        if (m_player1) storage.apply(m_player1, false);
        if (m_player2) storage.apply(m_player2, true);
    }
};