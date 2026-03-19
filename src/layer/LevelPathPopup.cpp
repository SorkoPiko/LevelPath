#include "LevelPathPopup.hpp"

#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>

#include "Geode/ui/GeodeUI.hpp"
#include "manager/AttemptStorage.hpp"
#include "manager/SaveQueue.hpp"
#include "node/LevelAttemptNode.hpp"
#include "serialise/Level.hpp"
#include "util/SerialiseUtils.hpp"

constexpr CCSize POPUP_SIZE = {400.0f, 250.0f};

bool LevelPathPopup::init(GJGameLevel* _level, LevelPath* _levelPath) {
    if (!FLAlertLayer::init(75)) return false;

    level = _level;
    levelPath = _levelPath;

    const CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    m_buttonMenu = Build<CCMenu>::create()
        .contentSize(POPUP_SIZE)
        .anchorPoint({0.5f, 0.5f})
        .ignoreAnchorPointForPos(false)
        .pos(winSize / 2)
        .id("menu")
        .parent(m_mainLayer);

    bg = Build<NineSlice>::create("GJ_square05.png")
        .contentSize(POPUP_SIZE)
        .zOrder(-1)
        .pos(winSize / 2)
        .id("bg")
        .parent(m_mainLayer);

    inner = Build<ListBorders>::create()
        .contentSize({POPUP_SIZE.width / 2.0f, POPUP_SIZE.height - 60.0f})
        .pos(winSize / 2 - CCSize{POPUP_SIZE.width / 4.0f - 15.0f, 15.0f})
        .id("inner")
        .zOrder(2)
        .parent(m_mainLayer);

    inner->setSprites(
        NineSlice::create("listTop.png"_spr),
        Build<NineSlice>::create("listTop.png"_spr).scaleY(-1),
        CCSprite::create("listSide.png"_spr),
        Build<CCSprite>::create("listSide.png"_spr).flipX(true),
        7.5f,
        7.5f
    );

    list = Build(cue::ListNode::create(
        inner->getContentSize() - CCSize{7.0f, 7.0f},
        {62, 62, 62, 255},
        cue::ListBorderStyle::None
    ))
        .zOrder(1)
        .id("list")
        .parent(m_mainLayer)
        .matchPos(inner);

    list->setCellColors(ccColor4B{0, 0, 0, 0}, ccColor4B{51, 51, 51, 255});

    Build(CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", 0.85f, CircleBaseColor::Gray))
        .scale(0.75f)
        .intoMenuItem([this] {
            onClose();
        })
        .pos({5.0f, POPUP_SIZE.height - 5.0f})
        .id("close-button")
        .parent(m_buttonMenu);

    Build(CircleButtonSprite::createWithSpriteFrameName("geode.loader/settings.png", 1.0f, CircleBaseColor::Gray))
        .scale(0.75f)
        .intoMenuItem([] {
            openSettingsPopup(Mod::get());
        })
        .pos({POPUP_SIZE.width - 5.0f, POPUP_SIZE.height - 5.0f})
        .id("settings-button")
        .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(
        "Level Paths",
        "bigFont.fnt"
    )
        .pos({POPUP_SIZE.width / 2.0f, POPUP_SIZE.height - 20.0f})
        .id("title")
        .parent(m_buttonMenu);

    attemptsLabel = Build<CCLabelBMFont>::create(
        "0 attempts",
        "chatFont.fnt"
    )
        .scale(0.8f)
        .anchorPoint({0.0f, 1.0f})
        .color({200, 200, 200})
        .id("attempts-label")
        .parent(m_mainLayer)
        .matchPos(inner)
        .move(inner->getScaledContentSize() / 2.0f + CCSize{10.0f, -10.0f});

    ticksLabel = Build<CCLabelBMFont>::create(
        "0 ticks stored",
        "chatFont.fnt"
    )
        .scale(0.8f)
        .anchorPoint({0.0f, 1.0f})
        .color({200, 200, 200})
        .id("size-label")
        .parent(m_mainLayer)
        .matchPos(attemptsLabel)
        .move({0.0f, -attemptsLabel->getScaledContentHeight() - 3.0f});

    sizeLabel = Build<CCLabelBMFont>::create(
        "0B on disk",
        "chatFont.fnt"
    )
        .scale(0.8f)
        .anchorPoint({0.0f, 1.0f})
        .color({200, 200, 200})
        .id("size-label")
        .parent(m_mainLayer)
        .matchPos(ticksLabel)
        .move({0.0f, -ticksLabel->getScaledContentHeight() - 3.0f});

    timeLabel = Build<CCLabelBMFont>::create(
        "0.0s recorded",
        "chatFont.fnt"
    )
        .scale(0.8f)
        .anchorPoint({0.0f, 1.0f})
        .color({200, 200, 200})
        .id("time-label")
        .parent(m_mainLayer)
        .matchPos(sizeLabel)
        .move({0.0f, -sizeLabel->getScaledContentHeight() - 3.0f});

    populateAttemptList();
    updateLabels();

    return true;
}

void LevelPathPopup::updateLabels() const {
    size_t attempts = levelPath->attempts.size();
    attemptsLabel->setString(fmt::format("{} attempt{}", attempts, attempts == 1 ? "" : "s").c_str());

    uint32_t ticks = 0;
    double totalTime = 0;
    Level serialisedLevel;
    for (const PathAttempt& attempt : levelPath->attempts) {
        serialisedLevel.attempts.push_back(SerialiseUtils::prepareForSerialisation(attempt));
        ticks += attempt.p1Ticks.size() + attempt.p2Ticks.size();
        totalTime += attempt.p1Ticks.size() / static_cast<float>(attempt.recordingRate);
    }
    ByteSize fileSize;
    ByteWriter writer;
    writer << serialisedLevel;
    fileSize.bytes = writer.buffer.size();
    ticksLabel->setString(fmt::format("{} tick{} stored", ticks, ticks == 1 ? "" : "s").c_str());
    sizeLabel->setString(fmt::format("{} on disk", fileSize).c_str());
    timeLabel->setString(fmt::format("{:.1f}s recorded", totalTime).c_str());
}

void LevelPathPopup::populateAttemptList() {
    list->clear();
    const CCSize cellSize = {list->getContentSize().width, 40.0f};
    for (size_t i = 0; i < levelPath->attempts.size(); ++i) {
        const auto& attempt = levelPath->attempts[i];
        const auto node = LevelAttemptNode::create(this, i, &attempt, cellSize);
        list->addCell(node);
    }
}

bool LevelPathPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    const CCPoint local = bg->convertToNodeSpace(touchLocation);
    const auto rect = CCRect(0, 0, bg->getContentWidth(), bg->getContentHeight());
    if (!rect.containsPoint(local)) {
        onClose();
        return true;
    }

    return true;
}
void LevelPathPopup::onClose() {
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}

LevelPathPopup* LevelPathPopup::create(GJGameLevel* level, LevelPath* levelPath) {
    auto ret = new LevelPathPopup();
    if (ret->init(level, levelPath)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void LevelPathPopup::show() {
    FLAlertLayer::show();
    handleTouchPriority(this);
}

void LevelPathPopup::deleteAttempt(const size_t index) {
    if (index >= levelPath->attempts.size()) return;

    levelPath->attempts.erase(levelPath->attempts.begin() + index);
    AttemptStorage::get().getSaveQueue().scheduleSave(fromLevel(level), *levelPath);

    size_t cellIndex = -1;
    for (const auto cell : list->iterChecked<LevelAttemptNode>()) {
        cellIndex++;
         if (cellIndex > index) {
            cell->updateIndex(cellIndex - 1);
        }
    }
    list->removeCell(index);
    list->updateLayout();
    updateLabels();
}