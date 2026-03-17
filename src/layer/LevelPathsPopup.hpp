#pragma once
#include <Geode/Prelude.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/utils/cocos.hpp>
#include <model/LevelPath.hpp>

using namespace geode::prelude;

class LevelPathsPopup : public FLAlertLayer {
    GJGameLevel* level = nullptr;
    std::optional<LevelPath> levelPath;

    Ref<NineSlice> bg = nullptr;
    Ref<ListBorders> inner = nullptr;
    Ref<CCMenu> tabMenu = nullptr;

    bool init(GJGameLevel* _level, std::optional<LevelPath> _levelPath);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;

    void onClose(CCObject*);

public:
    static LevelPathsPopup* create(GJGameLevel* level, std::optional<LevelPath> levelPath);
};