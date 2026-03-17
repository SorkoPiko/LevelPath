#include <Geode/Geode.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <UIBuilder.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <layer/LevelPathsPopup.hpp>

#include "LevelEditorLayer.hpp"

using namespace geode::prelude;

class $modify(EditorPauseLayer) {
    bool init(LevelEditorLayer* layer) {
        if (!EditorPauseLayer::init(layer)) return false;

        CCNode* menu = getChildByID("guidelines-menu");

        Build(CircleButtonSprite::createWithSpriteFrameName("GJ_lock_001.png", 1.0f, CircleBaseColor::Green, CircleBaseSize::Small))
            .intoMenuItem([] {
                openSettingsPopup(Mod::get());
            })
            .id("path-button"_spr)
            .parent(menu);

        menu->updateLayout();

        return true;
    }
};