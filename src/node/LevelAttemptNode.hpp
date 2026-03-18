#pragma once
#include "Geode/Prelude.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "layer/LevelPathPopup.hpp"
#include "model/PathAttempt.hpp"

using namespace geode::prelude;

class LevelAttemptNode : public CCNodeRGBA {
    LevelPathPopup* parentPopup = nullptr;

    size_t index = 0;
    const PathAttempt* attempt = nullptr;

    CCLabelBMFont* title = nullptr;
    CCLabelBMFont* tickLabel = nullptr;
    CCLabelBMFont* lengthLabel = nullptr;
    CCLabelBMFont* sizeLabel = nullptr;
    CCMenu* buttonMenu = nullptr;
    CCMenuItemSpriteExtra* deleteButton = nullptr;

    bool init(size_t _index, const PathAttempt* _attempt);

    void draw() override;

public:
    static LevelAttemptNode* create(LevelPathPopup* parent, size_t index, const PathAttempt* attempt, const CCSize &size);

    void updateIndex(int newIndex);
};