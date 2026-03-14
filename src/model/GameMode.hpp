#pragma once

#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameManager.hpp>

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

inline IconType getIconType(const GameMode mode) {
    switch (mode) {
        case GameMode::Cube:
            return IconType::Cube;
        case GameMode::Ship:
            return IconType::Ship;
        case GameMode::Ball:
            return IconType::Ball;
        case GameMode::UFO:
            return IconType::Ufo;
        case GameMode::Wave:
            return IconType::Wave;
        case GameMode::Robot:
            return IconType::Robot;
        case GameMode::Spider:
            return IconType::Spider;
        case GameMode::Swing:
            return IconType::Swing;
        default:
            return IconType::Cube;
    }
}

inline int getIconID(const GameMode mode) {
    GameManager* gameManager = GameManager::get();
    switch (mode) {
        case GameMode::Cube:
            return gameManager->getPlayerFrame();
        case GameMode::Ship:
            return gameManager->getPlayerShip();
        case GameMode::Ball:
            return gameManager->getPlayerBall();
        case GameMode::UFO:
            return gameManager->getPlayerBird();
        case GameMode::Wave:
            return gameManager->getPlayerDart();
        case GameMode::Robot:
            return gameManager->getPlayerRobot();
        case GameMode::Spider:
            return gameManager->getPlayerSpider();
        case GameMode::Swing:
            return gameManager->getPlayerSwing();
        default:
            return gameManager->getPlayerFrame();
    }
}