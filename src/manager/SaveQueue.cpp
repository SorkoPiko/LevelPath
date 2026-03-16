#include "SaveQueue.hpp"

#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>

#include <serialise/Level.hpp>

constexpr uint32_t MAGIC = 0x4C << 24 | 0x50 << 16 | 0x41 << 8 | 0x42;

std::filesystem::path getPath(const LevelIdentifier& levelID) {
    return dirs::getSaveDir() / "attempts" / fmt::format("{}.lpa", levelID);
}

SaveFuture SaveQueue::createSaveTask(const LevelIdentifier levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) {
    co_await createMigrationTask(levelID);

    const auto filePath = getPath(levelID);
    Result<ByteVector> existing = file::readBinary(filePath);
    Level level;
    if (existing) {
        ByteReader reader(*existing);
        uint32_t decodedMagic;
        reader >> decodedMagic;
        if (decodedMagic != MAGIC) {
            log::error("Failed to load existing data for level {}: invalid file type", levelID);
            std::filesystem::rename(filePath, filePath.string() + ".invalid");
        } else {
            reader >> level;
        }
    }

    Float16 lastX = Float16::fromFloat(0.0f);
    Float16 lastY = Float16::fromFloat(0.0f);
    Float16 lastRotation = Float16::fromFloat(0.0f);
    auto lastGameMode = GameMode::Cube;
    bool lastGravityFlipped = false;
    bool lastMini = false;

    std::vector<SerialisedAttemptTick> serialisedP1Ticks;
    for (const AttemptTick& tick : p1Ticks) {
        Float16 x = Float16::fromFloat(tick.x);
        Float16 y = Float16::fromFloat(tick.y);
        Float16 rotation = Float16::fromFloat(tick.rotation);

        serialisedP1Ticks.emplace_back(SerialisedAttemptTick{
            .x = x != lastX ? std::optional(x) : std::nullopt,
            .y = y != lastY ? std::optional(y) : std::nullopt,
            .rotation = rotation != lastRotation ? std::optional(rotation) : std::nullopt,
            .gameMode = tick.gameMode != lastGameMode ? std::optional(tick.gameMode) : std::nullopt,
            .gravityFlipped = tick.gravityFlipped != lastGravityFlipped ? std::optional(tick.gravityFlipped) : std::nullopt,
            .mini = tick.mini != lastMini ? std::optional(tick.mini) : std::nullopt
        });
        lastX = x;
        lastY = y;

        lastRotation = rotation;
        lastGameMode = tick.gameMode;
        lastGravityFlipped = tick.gravityFlipped;
        lastMini = tick.mini;
    }

    lastX = Float16::fromFloat(0.0f);
    lastY = Float16::fromFloat(0.0f);
    lastRotation = Float16::fromFloat(0.0f);
    lastGameMode = GameMode::Cube;
    lastGravityFlipped = false;
    lastMini = false;

    std::vector<SerialisedAttemptTick> serialisedP2Ticks;
    for (const AttemptTick& tick : p2Ticks) {
        Float16 x = Float16::fromFloat(tick.x);
        Float16 y = Float16::fromFloat(tick.y);
        Float16 rotation = Float16::fromFloat(tick.rotation);

        serialisedP2Ticks.emplace_back(SerialisedAttemptTick{
            .x = x != lastX ? std::optional(x) : std::nullopt,
            .y = y != lastY ? std::optional(y) : std::nullopt,
            .rotation = rotation != lastRotation ? std::optional(rotation) : std::nullopt,
            .gameMode = tick.gameMode != lastGameMode ? std::optional(tick.gameMode) : std::nullopt,
            .gravityFlipped = tick.gravityFlipped != lastGravityFlipped ? std::optional(tick.gravityFlipped) : std::nullopt,
            .mini = tick.mini != lastMini ? std::optional(tick.mini) : std::nullopt
        });

        lastX = x;
        lastY = y;
        lastRotation = rotation;
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

LoadFuture SaveQueue::createLoadTask(const LevelIdentifier levelID) {
    co_await createMigrationTask(levelID);

    const auto filePath = getPath(levelID);
    Result<ByteVector> existing = file::readBinary(filePath);
    if (!existing) {
        co_return std::nullopt;
    }
    ByteReader reader(*existing);
    uint32_t decodedMagic;
    reader >> decodedMagic;
    if (decodedMagic != MAGIC) {
        log::error("Failed to load existing data for level {}: invalid file type", levelID);
        std::filesystem::rename(filePath, filePath.string() + ".invalid");
        co_return std::nullopt;
    }
    Level rawLevel;
    reader >> rawLevel;

    LevelPath levelPath;

    for (const Attempt& attempt : rawLevel.attempts) {
        float lastX = 0.0f;
        float lastY = 0.0f;
        float lastRotation = 0.0f;
        auto lastGameMode = GameMode::Cube;
        bool lastGravityFlipped = false;
        bool lastMini = false;

        std::vector<AttemptTick> p1Ticks;
        for (const SerialisedAttemptTick& tick : attempt.p1Ticks) {
            if (tick.x) lastX = tick.x->toFloat();
            if (tick.y) lastY = tick.y->toFloat();
            if (tick.rotation) lastRotation = tick.rotation->toFloat();
            if (tick.gameMode) lastGameMode = *tick.gameMode;
            if (tick.gravityFlipped) lastGravityFlipped = *tick.gravityFlipped;
            if (tick.mini) lastMini = *tick.mini;

            p1Ticks.emplace_back(AttemptTick{
                .x = lastX,
                .y = lastY,
                .rotation = lastRotation,
                .gameMode = lastGameMode,
                .gravityFlipped = lastGravityFlipped,
                .mini = lastMini
            });
        }

        lastX = 0.0f;
        lastY = 0.0f;
        lastRotation = 0.0f;
        lastGameMode = GameMode::Cube;
        lastGravityFlipped = false;
        lastMini = false;

        std::vector<AttemptTick> p2Ticks;
        for (const SerialisedAttemptTick& tick : attempt.p2Ticks) {
            if (tick.x) lastX = tick.x->toFloat();
            if (tick.y) lastY = tick.y->toFloat();
            if (tick.rotation) lastRotation = tick.rotation->toFloat();
            if (tick.gameMode) lastGameMode = *tick.gameMode;
            if (tick.gravityFlipped) lastGravityFlipped = *tick.gravityFlipped;
            if (tick.mini) lastMini = *tick.mini;

            p2Ticks.emplace_back(AttemptTick{
                .x = lastX,
                .y = lastY,
                .rotation = lastRotation,
                .gameMode = lastGameMode,
                .gravityFlipped = lastGravityFlipped,
                .mini = lastMini
            });
        }

        levelPath.attempts.emplace_back(PathAttempt{
            .p1Ticks = std::move(p1Ticks),
            .p2Ticks = std::move(p2Ticks)
        });
    }

    co_return levelPath;
}

MigrationFuture SaveQueue::createMigrationTask(const LevelIdentifier levelID) {
    if (!levelID.editorID) co_return;

    LevelIdentifier oldID = {
        .levelID = *levelID.editorID,
        .levelType = GJLevelType::Editor,
        .editorID = std::nullopt
    };
    auto oldPath = getPath(oldID);
    auto newPath = getPath(levelID);

    Result<ByteVector> oldFile = file::readBinary(oldPath);
    if (!oldFile) co_return;

    Result<ByteVector> newFile = file::readBinary(newPath);
    if (!newFile) {
        std::filesystem::rename(oldPath, newPath);
        co_return;
    }

    ByteReader oldReader(*oldFile);
    ByteReader newReader(*newFile);

    Level oldLevel;
    Level newLevel;

    uint32_t decodedMagic;
    oldReader >> decodedMagic;
    if (decodedMagic != MAGIC) {
        log::error("Failed to load existing data for level {}: invalid file type", oldID);
        std::filesystem::rename(oldPath, oldPath.string() + ".invalid");
        co_return;
    }
    oldReader >> oldLevel;

    newReader >> decodedMagic;
    if (decodedMagic != MAGIC) {
        log::error("Failed to load existing data for level {}: invalid file type", levelID);
        std::filesystem::rename(newPath, newPath.string() + ".invalid");
        std::filesystem::rename(oldPath, newPath);
        co_return;
    }
    newReader >> newLevel;

    oldLevel.attempts.insert(
        oldLevel.attempts.end(),
        std::make_move_iterator(newLevel.attempts.begin()),
        std::make_move_iterator(newLevel.attempts.end())
    );

    ByteWriter writer;
    writer << MAGIC;
    writer << oldLevel;
    if (Result<> write = file::writeBinarySafe(newPath, writer.buffer); !write) {
        log::error("Failed to migrate attempts for level {}: {}", levelID, write.unwrapErr());
    }
    std::filesystem::remove(oldPath);

    co_return;
}

SaveQueue::SaveQueue() {
    auto [tx, rx] = arc::mpsc::channel<SaveFuture>();
    taskSender = tx;

    arc::spawn([rx = std::move(rx)]() mutable -> arc::Future<> {
        while (auto val = co_await rx.recv()) {
            auto fut = std::move(val).unwrap();
            try {
                co_await std::move(fut);
            } catch (const std::exception& e) {
                log::error("Error in save task: {}", e.what());
            }
        }
    });
}

void SaveQueue::scheduleSave(const LevelIdentifier levelID, std::vector<AttemptTick> p1Ticks, std::vector<AttemptTick> p2Ticks) const {
    auto task = createSaveTask(levelID, std::move(p1Ticks), std::move(p2Ticks));
    (void) taskSender->trySend(std::move(task));
}

void SaveQueue::scheduleLoad(const LevelIdentifier levelID, std::function<void(std::optional<LevelPath>)> callback) {
    async::spawn(
        createLoadTask(levelID),
        std::move(callback)
    );
}