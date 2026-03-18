#pragma once

#include <Geode/Enums.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

struct LevelIdentifier {
    int levelID;
    GJLevelType levelType;
    std::optional<int> editorID; // for migration

    std::string toString() const {
        return fmt::format("{}{}", getLevelTypeIdentifer(), levelID);
    }

    bool operator==(const LevelIdentifier& other) const {
        return levelID == other.levelID && getLevelTypeIdentifer() == other.getLevelTypeIdentifer();
    }

private:
    char getLevelTypeIdentifer() const {
        switch (levelType) {
            case GJLevelType::Editor:
                return 'e';
            case GJLevelType::Main:
                return 'm';
            default:
                return 'o';
        }
    }
};

inline std::string format_as(const LevelIdentifier& p) {
    return p.toString();
}

inline LevelIdentifier fromLevel(GJGameLevel* level) {
    int levelID = level->m_levelID.value();
    GJLevelType type = level->m_levelType;
    std::optional<int> editorID = std::nullopt;
    if (type == GJLevelType::Editor) {
        if (levelID > 0) {
            type = GJLevelType::Saved;
            const int storedEditorID = EditorIDs::getID(level);
            if (storedEditorID != levelID) editorID = storedEditorID;
        } else {
            levelID = EditorIDs::getID(level);
        }
    }
    return LevelIdentifier{levelID, type, editorID};
}