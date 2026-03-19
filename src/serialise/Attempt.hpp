#pragma once
#include "SerialisedAttemptTick.hpp"

struct Attempt {
    uint8_t recordingRate;
    bool fromStart;
    std::vector<SerialisedAttemptTick> p1Ticks;
    std::vector<SerialisedAttemptTick> p2Ticks;
};

inline void serialize(ByteWriter& writer, const Attempt& attempt) {
    writer << attempt.recordingRate;
    writer << attempt.fromStart;

    writer << static_cast<uint32_t>(attempt.p1Ticks.size());
    for (auto const& p1 : attempt.p1Ticks) {
        writer << p1;
    }

    writer << static_cast<uint32_t>(attempt.p2Ticks.size());
    for (auto const& p2 : attempt.p2Ticks) {
        writer << p2;
    }
}

inline void deserialize(ByteReader& reader, Attempt& attempt) {
    reader >> attempt.recordingRate;
    if (reader.context.version >= 2) reader >> attempt.fromStart;
    else attempt.fromStart = false;

    uint32_t p1Size;
    reader >> p1Size;
    attempt.p1Ticks.resize(p1Size);
    for (auto& p1 : attempt.p1Ticks) {
        reader >> p1;
    }

    uint32_t p2Size;
    reader >> p2Size;
    attempt.p2Ticks.resize(p2Size);
    for (auto& p2 : attempt.p2Ticks) {
        reader >> p2;
    }
}