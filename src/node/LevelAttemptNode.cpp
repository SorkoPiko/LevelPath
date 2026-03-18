#include "LevelAttemptNode.hpp"
#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>

#include "layer/LevelPathPopup.hpp"
#include "util/ByteUtils.hpp"
#include "util/SerialiseUtils.hpp"

bool LevelAttemptNode::init(const size_t _index, const PathAttempt* _attempt) {
    if (!CCNodeRGBA::init()) return false;

    index = _index;
    attempt = _attempt;

    title = Build<CCLabelBMFont>::create(
        fmt::format("Attempt {}", index + 1).c_str(),
        "bigFont.fnt"
    )
        .scale(0.5f)
        .anchorPoint({0.0f, 1.0f})
        .pos({5.0f, getContentHeight() - 2.5f})
        .id("title")
        .parent(this);

    tickLabel = Build<CCLabelBMFont>::create(
        fmt::format("{} ticks", attempt->p1Ticks.size()).c_str(),
        "chatFont.fnt"
    )
        .scale(0.5f)
        .anchorPoint({0.0f, 1.0f})
        .color({150, 150, 150})
        .id("tick-label")
        .parent(this)
        .matchPos(title)
        .move({0.0f, -title->getScaledContentHeight() - 2.0f});

    const float tickLength = 1.0f / attempt->recordingRate;
    lengthLabel = Build<CCLabelBMFont>::create(
        fmt::format("{:.1f}s", tickLength * attempt->p1Ticks.size()).c_str(),
        "chatFont.fnt"
    )
        .scale(0.5f)
        .anchorPoint({0.0f, 1.0f})
        .color({150, 150, 150})
        .id("length-label")
        .parent(this)
        .matchPos(tickLabel)
        .move({0.0f, -tickLabel->getScaledContentHeight()});

    ByteSize size;
    ByteWriter writer;
    writer << SerialiseUtils::prepareForSerialisation(*attempt);
    size.bytes = writer.buffer.size();
    sizeLabel = Build<CCLabelBMFont>::create(
        fmt::format("{} on disk", size).c_str(),
        "chatFont.fnt"
    )
        .scale(0.5f)
        .anchorPoint({0.0f, 1.0f})
        .color({150, 150, 150})
        .id("size-label")
        .parent(this)
        .matchPos(tickLabel)
        .move({std::max(tickLabel->getScaledContentWidth(), lengthLabel->getScaledContentWidth()) + 8.0f, 0.0f});

    buttonMenu = Build<CCMenu>::create()
        .pos({0.0f, 0.0f})
        .parent(this);

    deleteButton = Build<CCSprite>::createSpriteName("GJ_deleteBtn_001.png")
        .scale(0.5f)
        .intoMenuItem([this] {
            parentPopup->deleteAttempt(index);
        })
        .pos({getContentWidth() - 20.0f, getContentHeight() / 2.0f})
        .id("delete-button")
        .parent(buttonMenu);

    return true;
}

void LevelAttemptNode::draw() {
    CCNodeRGBA::draw();

    cocos2d::ccDrawColor4B(0, 0, 0, 80);
    glLineWidth(2.0f);
    float lineX = std::max(
        tickLabel->getPositionX() + tickLabel->getScaledContentWidth(),
        lengthLabel->getPositionX() + lengthLabel->getScaledContentWidth()
    ) + 4.0f;
    ccDrawLine({
        lineX, lengthLabel->getPositionY() - lengthLabel->getScaledContentHeight()},
        {lineX, tickLabel->getPositionY()}
    );
}

LevelAttemptNode* LevelAttemptNode::create(LevelPathPopup* parent, const size_t index, const PathAttempt* attempt, const CCSize &size) {
    auto ret = new LevelAttemptNode();
    ret->parentPopup = parent;
    ret->setContentSize(size);
    if (ret->init(index, attempt)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void LevelAttemptNode::updateIndex(const int newIndex) {
    index = newIndex;
    title->setString(fmt::format("Attempt {}", index + 1).c_str());
}