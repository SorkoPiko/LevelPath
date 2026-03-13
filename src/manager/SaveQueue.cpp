#include "SaveQueue.hpp"

#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>

#include <serialise/Level.hpp>

constexpr uint32_t MAGIC = 0x4C << 24 | 0x50 << 16 | 0x41 << 8 | 0x42;

SaveFuture SaveQueue::createSaveTask(const int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) {
    const auto filePath = dirs::getSaveDir() / "attempts" / fmt::format("{}.lpa", levelID);
    Result<ByteVector> existing = file::readBinary(filePath);
    Level level;
    if (existing) {
        ByteReader reader(*existing);
        uint32_t decodedMagic;
        reader >> decodedMagic;
        if (decodedMagic != MAGIC) {
            log::error("Failed to load existing attempt for level {}: invalid file type", levelID);
            std::filesystem::rename(filePath, filePath.string() + ".invalid");
        } else {
            reader >> level;    
        }
    }

    float lastY = 0.0f;
    float lastRotation = 0.0f;
    auto lastGameMode = GameMode::Cube;
    bool lastGravityFlipped = false;
    bool lastMini = false;

    std::vector<SerialisedAttemptTick> serialisedP1Ticks;
    for (const AttemptTick& tick : p1Ticks) {
        serialisedP1Ticks.emplace_back(SerialisedAttemptTick{
            .x = Float16::fromFloat(tick.x),
            .y = tick.y != lastY ? std::optional(Float16::fromFloat(tick.y)) : std::nullopt,
            .rotation = tick.rotation != lastRotation ? std::optional(Float16::fromFloat(tick.rotation)) : std::nullopt,
            .gameMode = tick.gameMode != lastGameMode ? std::optional(tick.gameMode) : std::nullopt,
            .gravityFlipped = tick.gravityFlipped != lastGravityFlipped ? std::optional(tick.gravityFlipped) : std::nullopt,
            .mini = tick.mini != lastMini ? std::optional(tick.mini) : std::nullopt
        });
        lastY = tick.y;
        lastRotation = tick.rotation;
        lastGameMode = tick.gameMode;
        lastGravityFlipped = tick.gravityFlipped;
        lastMini = tick.mini;
    }

    lastY = 0.0f;
    lastRotation = 0.0f;
    lastGameMode = GameMode::Cube;
    lastGravityFlipped = false;
    lastMini = false;

    std::vector<SerialisedAttemptTick> serialisedP2Ticks;
    for (const AttemptTick& tick : p2Ticks) {
        serialisedP1Ticks.emplace_back(SerialisedAttemptTick{
            .x = Float16::fromFloat(tick.x),
            .y = tick.y != lastY ? std::optional(Float16::fromFloat(tick.y)) : std::nullopt,
            .rotation = tick.rotation != lastRotation ? std::optional(Float16::fromFloat(tick.rotation)) : std::nullopt,
            .gameMode = tick.gameMode != lastGameMode ? std::optional(tick.gameMode) : std::nullopt,
            .gravityFlipped = tick.gravityFlipped != lastGravityFlipped ? std::optional(tick.gravityFlipped) : std::nullopt,
            .mini = tick.mini != lastMini ? std::optional(tick.mini) : std::nullopt
        });
        lastY = tick.y;
        lastRotation = tick.rotation;
        lastGameMode = tick.gameMode;
        lastGravityFlipped = tick.gravityFlipped;
        lastMini = tick.mini;
    }

    level.attempts.emplace_back(Attempt{
        .p1Ticks = std::move(serialisedP1Ticks),
        .p2Ticks = std::move(serialisedP2Ticks)
    });

    ByteWriter writer;
    writer << MAGIC;
    writer << level;
    if (Result<> write = file::writeBinarySafe(filePath, writer.buffer); !write) {
        log::error("Failed to save attempt for level {}: {}", levelID, write.unwrapErr());
    }

    co_return;
}

SaveQueue::SaveQueue() {
    auto [tx, rx] = arc::mpsc::channel<SaveFuture>();
    taskSender = tx;

    arc::spawn([rx = std::move(rx)]() mutable -> arc::Future<> {
        while (true) {
            auto val = co_await rx.recv();
            if (!val) break;

            auto fut = std::move(val).unwrap();
            try {
                co_await std::move(fut);
            } catch (const std::exception& e) {
                log::error("Error in save task: {}", e.what());
            }
        }
    });
}

void SaveQueue::scheduleSave(const int levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) const {
    auto task = createSaveTask(levelID, std::move(p1Ticks), std::move(p2Ticks));
    (void) taskSender->trySend(std::move(task));
}