#pragma once
#include <Geode/Prelude.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/utils/cocos.hpp>
#include <model/LevelPath.hpp>

#include "cue/ListNode.hpp"

using namespace geode::prelude;

class LevelPathPopup : public FLAlertLayer {
    GJGameLevel* level = nullptr;
    LevelPath* levelPath = nullptr;

    Ref<NineSlice> bg = nullptr;
    Ref<ListBorders> inner = nullptr;
    Ref<cue::ListNode> list = nullptr;

    Ref<CCLabelBMFont> attemptsLabel = nullptr;

    bool init(GJGameLevel* _level, LevelPath* _levelPath);

    void populateAttemptList();

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;

    void onClose();

public:
    static LevelPathPopup* create(GJGameLevel* level, LevelPath* levelPath);

    void show() override;

    void deleteAttempt(size_t index);
};