#include "SaveQueue.hpp"

#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>

#include <serialise/Level.hpp>
#include "util/SerialiseUtils.hpp"

constexpr uint32_t MAGIC = 0x4C << 24 | 0x50 << 16 | 0x41 << 8 | 0x42;

std::filesystem::path getPath(const LevelIdentifier& levelID) {
    return dirs::getSaveDir() / "attempts" / fmt::format("{}.lpa", levelID);
}

SaveFuture SaveQueue::createAppendTask(const LevelIdentifier levelID, PathAttempt attempt) {
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

    level.attempts.emplace_back(SerialiseUtils::prepareForSerialisation(attempt));

    ByteWriter writer;
    writer << MAGIC;
    writer << level;
    if (Result<> write = file::writeBinarySafe(filePath, writer.buffer); !write) {
        log::error("Failed to save attempt for level {}: {}", levelID, write.unwrapErr());
    }

    co_return;
}

SaveFuture SaveQueue::createSaveTask(const LevelIdentifier levelID, LevelPath levelPath) {
    const auto filePath = getPath(levelID);
    Level level;

    for (const PathAttempt& attempt : levelPath.attempts) {
        level.attempts.emplace_back(SerialiseUtils::prepareForSerialisation(attempt));
    }

    ByteWriter writer;
    writer << MAGIC;
    writer << level;
    if (Result<> write = file::writeBinarySafe(filePath, writer.buffer); !write) {
        log::error("Failed to save level {}: {}", levelID, write.unwrapErr());
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
            .recordingRate = attempt.recordingRate,
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

void SaveQueue::scheduleAppend(const LevelIdentifier levelID, PathAttempt attempt) const {
    auto task = createAppendTask(levelID, std::move(attempt));
    (void) taskSender->trySend(std::move(task));
}

void SaveQueue::scheduleSave(const LevelIdentifier levelID, LevelPath levelPath) const {
    auto task = createSaveTask(levelID, std::move(levelPath));
    (void) taskSender->trySend(std::move(task));
}

void SaveQueue::scheduleLoad(const LevelIdentifier levelID, std::function<void(std::optional<LevelPath>)> callback) {
    async::spawn(
        createLoadTask(levelID),
        std::move(callback)
    );
}