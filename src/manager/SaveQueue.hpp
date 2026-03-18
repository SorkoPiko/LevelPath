#pragma once

#include <arc/prelude.hpp>
#include <model/LevelIdentifier.hpp>
#include <model/LevelPath.hpp>

using SaveFuture = arc::Future<>;
using LoadFuture = arc::Future<std::optional<LevelPath>>;
using MigrationFuture = arc::Future<>;

class SaveQueue {
    std::optional<arc::mpsc::Sender<SaveFuture>> taskSender;

    static SaveFuture createAppendTask(LevelIdentifier levelID, PathAttempt attempt);
    static SaveFuture createSaveTask(LevelIdentifier levelID, LevelPath levelPath);
    static LoadFuture createLoadTask(LevelIdentifier levelID);
    static MigrationFuture createMigrationTask(LevelIdentifier levelID);

public:
    SaveQueue();

    void scheduleAppend(LevelIdentifier levelID, PathAttempt attempt) const;
    void scheduleSave(LevelIdentifier levelID, LevelPath levelPath) const;
    static void scheduleLoad(LevelIdentifier levelID, std::function<void(std::optional<LevelPath>)> callback);
};