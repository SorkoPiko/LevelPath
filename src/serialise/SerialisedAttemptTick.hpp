#pragma once
#include <model/AttemptTick.hpp>
#include <util/ByteUtils.hpp>
#include <stdfloat>

struct SerialisedAttemptTick {
    float x;
    std::optional<float> y;
    std::optional<float> rotation;
    std::optional<GameMode> gameMode;
    std::optional<bool> gravityFlipped;
    std::optional<bool> mini;
};

inline void serialize(ByteWriter& writer, SerialisedAttemptTick const& tick) {
    writer << tick.x;

    std::float_t a;
    uint8_t changed_mask = 0;
    if (tick.y) changed_mask |= 1 << 0;
    if (tick.rotation) changed_mask |= 1 << 1;
    if (tick.gameMode) changed_mask |= 1 << 2;
    if (tick.gravityFlipped) changed_mask |= 1 << 3;
    if (tick.mini) changed_mask |= 1 << 4;
    writer << changed_mask;

    if (tick.y) writer << *tick.y;
    if (tick.rotation) writer << *tick.rotation;

    uint8_t packed_byte = 0;
    if (tick.gameMode) {
        packed_byte |= static_cast<uint8_t>(*tick.gameMode) & 0x7;
    }
    if (tick.gravityFlipped && *tick.gravityFlipped) {
        packed_byte |= 1 << 3;
    }
    if (tick.mini && *tick.mini) {
        packed_byte |= 1 << 4;
    }
    writer << packed_byte;
}

inline void deserialize(ByteReader& reader, SerialisedAttemptTick& tick) {
    reader >> tick.x;

    uint8_t changed_mask;
    reader >> changed_mask;

    if (changed_mask & 1 << 0) reader >> tick.y.emplace();
    if (changed_mask & 1 << 1) reader >> tick.rotation.emplace();

    uint8_t packed_byte;
    reader >> packed_byte;

    if (changed_mask & 1 << 2) {
        auto gameMode_value = static_cast<GameMode>(packed_byte & 0x7);
        tick.gameMode = gameMode_value;
    }

    if (changed_mask & 1 << 3) {
        tick.gravityFlipped = packed_byte & 1 << 3;
    }
    if (changed_mask & 1 << 4) {
        tick.mini = packed_byte & 1 << 4;
    }
}