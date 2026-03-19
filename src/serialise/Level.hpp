#pragma once
#include "Attempt.hpp"

struct Level {
    std::vector<Attempt> attempts;
};

inline void serialize(ByteWriter& writer, const Level& level) {
    writer << static_cast<uint8_t>(2); // version

    writer << static_cast<uint32_t>(level.attempts.size());
    for (auto const& attempt : level.attempts) {
        writer << attempt;
    }
}

inline void deserialize(ByteReader& reader, Level& level) {
    uint8_t version;
    reader >> version;
    reader.context.version = version;

    uint32_t attemptsSize;
    reader >> attemptsSize;
    level.attempts.resize(attemptsSize);
    for (auto& attempt : level.attempts) {
        reader >> attempt;
    }
}