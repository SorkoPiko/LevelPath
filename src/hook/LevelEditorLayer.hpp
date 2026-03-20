#pragma once
#include <Geode/Prelude.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <manager/AttemptStorage.hpp>

using namespace geode::prelude;

class $modify(LPLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        CCRenderTexture* pathNode = nullptr;
        std::optional<LevelPath> currentPath;
        std::vector<bool> shownAttempts;
        std::unordered_map<const AttemptTick*, float> nearbyCache;

        CCAffineTransform lastTransform = CCAffineTransformMakeIdentity();
        float lastOpacity = -1.0f;
        size_t lastShownHash = 0;
    };

    bool init(GJGameLevel* level, bool noUI);

    void drawPath(float);
    void buildPathCache();
};