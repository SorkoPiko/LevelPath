#pragma once

#include <arc/prelude.hpp>
#include <model/AttemptTick.hpp>

using SaveFuture = arc::Future<>;

class SaveQueue {
    std::optional<arc::mpsc::Sender<SaveFuture>> taskSender;

    static SaveFuture createSaveTask(int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks);

public:
    SaveQueue();

    void scheduleSave(int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) const;
};
