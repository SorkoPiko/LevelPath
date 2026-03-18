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

    Ref<CCLabelBMFont> title = nullptr;
    Ref<CCLabelBMFont> tickLabel = nullptr;
    Ref<CCLabelBMFont> lengthLabel = nullptr;
    Ref<CCLabelBMFont> sizeLabel = nullptr;
    Ref<CCMenu> buttonMenu = nullptr;
    Ref<CCMenuItemSpriteExtra> deleteButton = nullptr;

    bool init(size_t _index, const PathAttempt* _attempt);

    void draw() override;

public:
    static LevelAttemptNode* create(LevelPathPopup* parent, size_t index, const PathAttempt* attempt, const CCSize &size);

    void updateIndex(int newIndex);
};