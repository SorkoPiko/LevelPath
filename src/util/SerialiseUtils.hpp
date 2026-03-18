#pragma once
#include "model/PathAttempt.hpp"
#include "serialise/Attempt.hpp"

class SerialiseUtils {
    SerialiseUtils() = delete;
public:
    static Attempt prepareForSerialisation(const PathAttempt& attempt) {
        Float16 lastX = Float16::fromFloat(0.0f);
        Float16 lastY = Float16::fromFloat(0.0f);
        Float16 lastRotation = Float16::fromFloat(0.0f);
        auto lastGameMode = GameMode::Cube;
        bool lastGravityFlipped = false;
        bool lastMini = false;

        std::vector<SerialisedAttemptTick> serialisedP1Ticks;
        for (const AttemptTick& tick : attempt.p1Ticks) {
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
        for (const AttemptTick& tick : attempt.p2Ticks) {
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

        return Attempt{
            .recordingRate = static_cast<uint8_t>(attempt.recordingRate),
            .p1Ticks = std::move(serialisedP1Ticks),
            .p2Ticks = std::move(serialisedP2Ticks)
        };
    }
};
