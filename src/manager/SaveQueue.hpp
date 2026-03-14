#pragma once

#include <arc/prelude.hpp>
#include <model/LevelPath.hpp>

using SaveFuture = arc::Future<>;
using LoadFuture = arc::Future<std::optional<LevelPath>>;

class SaveQueue {
    std::optional<arc::mpsc::Sender<SaveFuture>> taskSender;

    static SaveFuture createSaveTask(int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks);
    static LoadFuture createLoadTask(int levelID);

public:
    SaveQueue();

    void scheduleSave(int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) const;
    static void scheduleLoad(int levelID, std::function<void(std::optional<LevelPath>)> callback);
};