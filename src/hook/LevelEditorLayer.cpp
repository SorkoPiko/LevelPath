#include <Geode/Geode.hpp>
#include <Geode/Prelude.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <UIBuilder.hpp>
#include <manager/AttemptStorage.hpp>

using namespace geode::prelude;

void updateIconType(PlayerObject* player, const GameMode gameMode) {
    GameManager* gameManager = GameManager::get();

    player->toggleFlyMode(gameMode == GameMode::Ship, true);
    player->toggleRollMode(gameMode == GameMode::Ball, true);
    player->toggleBirdMode(gameMode == GameMode::UFO, true);
    player->toggleDartMode(gameMode == GameMode::Wave, true);
    player->toggleRobotMode(gameMode == GameMode::Robot, true);
    player->toggleSpiderMode(gameMode == GameMode::Spider, true);
    player->toggleSwingMode(gameMode == GameMode::Swing, true);

    switch (gameMode) {
        case GameMode::Cube:
            break;
        case GameMode::Ship:
            player->updatePlayerShipFrame(gameManager->getPlayerShip());
            break;
        case GameMode::Ball:
            player->updatePlayerRollFrame(gameManager->getPlayerBall());
            break;
        case GameMode::UFO:
            player->updatePlayerBirdFrame(gameManager->getPlayerBird());
            break;
        case GameMode::Wave:
            player->updatePlayerDartFrame(gameManager->getPlayerDart());
            break;
        case GameMode::Robot:
            player->updatePlayerRobotFrame(gameManager->getPlayerRobot());
            break;
        case GameMode::Spider:
            player->updatePlayerSpiderFrame(gameManager->getPlayerSpider());
            break;
        case GameMode::Swing:
            player->updatePlayerSwingFrame(gameManager->getPlayerSwing());
            break;
    }

    if (gameMode == GameMode::Cube || gameMode == GameMode::Ship || gameMode == GameMode::UFO) {
        player->updatePlayerFrame(gameManager->getPlayerFrame());
    }

    player->updateGlowColor();
    player->updatePlayerGlow();
}

class $modify(LevelEditorLayer) {
    struct Fields {
        CCRenderTexture* pathNode;
        LevelPath currentPath;
    };

    bool init(GJGameLevel* level, const bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;

        CCNode* parent = m_debugDrawNode->getParent();
        m_fields->pathNode = Build<CCRenderTexture>::create(
            parent->getContentWidth(),
            parent->getContentHeight(),
            kCCTexture2DPixelFormat_RGBA8888,
            GL_DEPTH24_STENCIL8
        )
            .id("level-path-node"_spr)
            .zOrder(1500)
            .parent(parent);
        m_fields->pathNode->getSprite()->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        m_fields->pathNode->getSprite()->getTexture()->setAntiAliasTexParameters();

        AttemptStorage::get().getSaveQueue().scheduleLoad(fromLevel(m_level), [this](const std::optional<LevelPath>& path) {
            if (!path) return;

            m_fields->currentPath = std::move(*path);
            drawPath();
        });

        return true;
    }

    void drawPath() {
        GameManager* gameManager = GameManager::get();
        SimplePlayer* player = SimplePlayer::create(0);

        player->setColor(gameManager->colorForIdx(gameManager->getPlayerColor()));
        player->setSecondColor(gameManager->colorForIdx(gameManager->getPlayerColor2()));

        if (gameManager->getPlayerGlow()) {
            player->setGlowOutline(gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
        } else {
            player->disableGlowOutline();
        }

        CCSprite* test = CCSprite::createWithSpriteFrameName("GJ_normalBtn_001.png");

        m_fields->pathNode->beginWithClear(0.0, 0.0, 0.0, 0.0);
        for (const PathAttempt& attempt : m_fields->currentPath.attempts) {
            for (const AttemptTick& tick : attempt.p1Ticks) {
                log::debug("Drawing at {} {}", tick.x, tick.y);
                player->setPosition({tick.x, tick.y});
                test->setPosition({tick.x, tick.y});
                player->setRotation(tick.rotation);
                player->updatePlayerFrame(getIconID(tick.gameMode), getIconType(tick.gameMode));
                player->setScale(tick.mini ? 0.6f : 1.0f);
                player->setScaleY(tick.gravityFlipped ? -std::abs(player->getScaleY()) : std::abs(player->getScaleY()));

                player->visit();
                test->visit();
            }
        }

        m_fields->pathNode->end();
    }
};