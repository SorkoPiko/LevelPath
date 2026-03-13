#pragma once
#include "Attempt.hpp"

struct Level {
    std::vector<Attempt> attempts;
};

inline std::string magic = "LPAB";

inline void serialize(ByteWriter& writer, const Level& level) {
    writer << magic;
    writer << static_cast<uint8_t>(1);
    writer << static_cast<uint32_t>(level.attempts.size());
    for (auto const& attempt : level.attempts) {
        writer << attempt;
    }
}

inline void deserialize_v1(ByteReader& reader, Level& level) {
    uint32_t attemptsSize;
    reader >> attemptsSize;
    level.attempts.resize(attemptsSize);
    for (auto& attempt : level.attempts) {
        reader >> attempt;
    }
}

inline void deserialize(ByteReader& reader, Level& level) {
    std::string decodedMagic;
    reader >> decodedMagic;
    if (decodedMagic != magic) {
        throw std::runtime_error("Invalid file format");
    }

    uint8_t version;
    reader >> version;
    switch (version) {
        case 1:
            deserialize_v1(reader, level);
            break;
        default:
            throw std::runtime_error("Unsupported file version");
    }
}