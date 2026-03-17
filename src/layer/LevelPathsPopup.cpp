#include "LevelPathsPopup.hpp"

#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>

constexpr CCSize POPUP_SIZE = {400.0f, 250.0f};

bool LevelPathsPopup::init(GJGameLevel* _level, std::optional<LevelPath> _levelPath) {
    if (!FLAlertLayer::init(0)) return false;

    level = _level;
    levelPath = std::move(_levelPath);

    const CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    bg = Build<NineSlice>::create("GJ_square05.png")
        .contentSize(POPUP_SIZE)
        .zOrder(-1)
        .pos(winSize / 2)
        .id("bg")
        .parent(m_mainLayer);

    inner = Build<ListBorders>::create()
        .contentSize(POPUP_SIZE - CCSize{30.0f, 60.0f})
        .pos(winSize / 2 - CCSize{0.0f, 15.0f})
        .id("inner")
        .parent(m_mainLayer);

    inner->setSprites(
        NineSlice::create("listTop.png"_spr),
        Build<NineSlice>::create("listTop.png"_spr).scaleY(-1),
        CCSprite::create("listSide.png"_spr),
        Build<CCSprite>::create("listSide.png"_spr).flipX(true),
        7.5f,
        7.5f
    );

    tabMenu = Build<CCMenu>::create()
        .contentSize(POPUP_SIZE.width - 30.0f, 30.0f)
        .anchorPoint({0.5f, 1.0f})
        .ignoreAnchorPointForPos(false)
        .pos(winSize / 2 + CCSize{0.0f, POPUP_SIZE.height / 2 - 10.0f})
        .id("tab-menu")
        .parent(m_mainLayer);

    return true;
}

bool LevelPathsPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    const CCPoint local = bg->convertToNodeSpace(touchLocation);
    const auto rect = CCRect(0, 0, bg->getContentWidth(), bg->getContentHeight());
    if (!rect.containsPoint(local)) {
        onClose(nullptr);
        return true;
    }

    return true;
}

void LevelPathsPopup::onClose(CCObject*) {
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}

LevelPathsPopup* LevelPathsPopup::create(GJGameLevel* level, std::optional<LevelPath> levelPath) {
    auto ret = new LevelPathsPopup();
    if (ret->init(level, std::move(levelPath))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}