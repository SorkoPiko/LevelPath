#pragma once

#include <Geode/binding/PlayerObject.hpp>

enum class GameMode {
    Cube,
    Ship,
    Ball,
    UFO,
    Wave,
    Robot,
    Spider,
    Swing
};

inline GameMode fromPlayer(const PlayerObject* player) {
    if (!player) return GameMode::Cube;

    if (player->m_isShip) return GameMode::Ship;
    if (player->m_isBall) return GameMode::Ball;
    if (player->m_isBird) return GameMode::UFO;
    if (player->m_isDart) return GameMode::Wave;
    if (player->m_isRobot) return GameMode::Robot;
    if (player->m_isSpider) return GameMode::Spider;
    if (player->m_isSwing) return GameMode::Swing;
    return GameMode::Cube;
}