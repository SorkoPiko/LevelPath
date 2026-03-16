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

bool transformEquals(const CCAffineTransform& a, const CCAffineTransform& b) {
    return a.a == b.a && a.b == b.b && a.c == b.c && a.d == b.d && a.tx == b.tx && a.ty == b.ty;
}

float compositionAlpha(const float finalAlpha, const float count) {
    return 1 - std::pow(1 - finalAlpha, 1.0f / count);
}

class $modify(LPLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        CCRenderTexture* pathNode = nullptr;
        std::optional<LevelPath> currentPath;
        std::unordered_map<const AttemptTick*, float> nearbyCache;
        CCAffineTransform lastTransform = CCAffineTransformMakeIdentity();
    };

    bool init(GJGameLevel* level, const bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;

        const auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_fields->pathNode = Build<CCRenderTexture>::create(
            winSize.width, winSize.height,
            kCCTexture2DPixelFormat_RGBA8888,
            GL_DEPTH24_STENCIL8
        )
            .pos(winSize.width / 2, winSize.height / 2)
            .id("level-path-node"_spr)
            .parent(this);
        m_fields->pathNode->getSprite()->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        m_fields->pathNode->getSprite()->getTexture()->setAntiAliasTexParameters();

        SaveQueue::scheduleLoad(fromLevel(m_level), [this](const std::optional<LevelPath>& path) {
            if (!path) return;

            log::debug("Loaded path with {} attempts", path->attempts.size());
            m_fields->currentPath = *path;
            buildPathCache();
        });

        schedule(schedule_selector(LPLevelEditorLayer::drawPath), 0.0f, kCCRepeatForever, 0.0f);

        return true;
    }

    void drawPath(float) {
        if (!m_fields->currentPath) return;

        if (transformEquals(m_objectLayer->nodeToWorldTransform(), m_fields->lastTransform)) return;
        m_fields->lastTransform = m_objectLayer->nodeToWorldTransform();

        const LevelPath& path = *m_fields->currentPath;

        GameManager* gameManager = GameManager::get();
        PlayerObject* player = PlayerObject::create(1, 1, this, m_objectLayer, false);

        const ccColor3B primary = gameManager->colorForIdx(gameManager->getPlayerColor());
        const ccColor3B secondary = gameManager->colorForIdx(gameManager->getPlayerColor2());
        player->setColor(secondary);
        player->setSecondColor(primary);

        if (gameManager->getPlayerGlow()) {
            player->m_hasGlow = true;
            player->enableCustomGlowColor(gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
        } else {
            player->m_hasGlow = false;
            player->disableCustomGlowColor();
        }

        const CCSize spriteSize = m_fields->pathNode->getSprite()->getContentSize();
        m_fields->pathNode->beginWithClear(0.0, 0.0, 0.0, 0.0);

        auto lastGameMode = GameMode::Cube;
        updateIconType(player, lastGameMode);

        for (const PathAttempt& attempt : path.attempts) {
            for (const AttemptTick& tick : attempt.p2Ticks) {
                CCPoint pos = CCPointApplyAffineTransform({tick.x, tick.y}, m_fields->lastTransform);
                if (pos.x < -spriteSize.width || pos.y < -spriteSize.height || pos.x > spriteSize.width * 2.0f || pos.y > spriteSize.height * 2.0f) continue;
                player->setPosition(pos);
                player->setRotation(tick.rotation);
                if (tick.gameMode != lastGameMode) {
                    lastGameMode = tick.gameMode;
                    updateIconType(player, lastGameMode);
                }
                const CCSize scale = CCSizeApplyAffineTransform({1.0f, 1.0f}, m_fields->lastTransform) * (tick.mini ? 0.6f : 1.0f);
                player->setScaleX(scale.width);
                player->setScaleY(tick.gravityFlipped ? -scale.height : scale.height);

                const CCSize content = scale * 40.0f;
                if (pos.x < 0 - content.width || pos.y < 0 - content.height || pos.x > spriteSize.width + content.width || pos.y > spriteSize.height + content.height) continue;

                player->setOpacity(compositionAlpha(0.5f, m_fields->nearbyCache[&tick]) * 255.0f);
                player->visit();
            }
        }

        player->setColor(primary);
        player->setSecondColor(secondary);

        for (const PathAttempt& attempt : path.attempts) {
            for (const AttemptTick& tick : attempt.p1Ticks) {
                CCPoint pos = CCPointApplyAffineTransform({tick.x, tick.y}, m_fields->lastTransform);
                if (pos.x < -spriteSize.width || pos.y < -spriteSize.height || pos.x > spriteSize.width * 2.0f || pos.y > spriteSize.height * 2.0f) continue;
                player->setPosition(pos);
                player->setRotation(tick.rotation);
                if (tick.gameMode != lastGameMode) {
                    lastGameMode = tick.gameMode;
                    updateIconType(player, lastGameMode);
                }
                const CCSize scale = CCSizeApplyAffineTransform({1.0f, 1.0f}, m_fields->lastTransform) * (tick.mini ? 0.6f : 1.0f);
                player->setScaleX(scale.width);
                player->setScaleY(tick.gravityFlipped ? -scale.height : scale.height);

                const CCSize content = scale * 40.0f;
                if (pos.x < 0 - content.width || pos.y < 0 - content.height || pos.x > spriteSize.width + content.width || pos.y > spriteSize.height + content.height) continue;

                player->setOpacity(compositionAlpha(0.5f, m_fields->nearbyCache[&tick]) * 255.0f);
                player->visit();
            }
        }

        m_fields->pathNode->end();

        player->removeFromParentAndCleanup(true);
    }

    void buildPathCache() {
        if (!m_fields->currentPath) return;

        const LevelPath& path = *m_fields->currentPath;

        std::vector<const AttemptTick*> allTicks;
        for (const PathAttempt& attempt : path.attempts) {
            for (const AttemptTick& tick : attempt.p1Ticks) {
                allTicks.push_back(&tick);
            }
            for (const AttemptTick& tick : attempt.p2Ticks) {
                allTicks.push_back(&tick);
            }
        }

        constexpr float PROXIMITY_THRESHOLD = 25.0f;
        for (const AttemptTick* tick : allTicks) {
            float count = 1;
            for (const AttemptTick* otherTick : allTicks) {
                if (otherTick == tick) continue;
                const float dx = tick->x - otherTick->x;
                const float dy = tick->y - otherTick->y;
                const float distance = sqrt(dx * dx + dy * dy) / (tick->mini ? 0.6f : 1.0f);
                if (distance <= PROXIMITY_THRESHOLD) {
                    count += (PROXIMITY_THRESHOLD - distance) / PROXIMITY_THRESHOLD;
                }
            }
            m_fields->nearbyCache[tick] = count;
        }
    }
};