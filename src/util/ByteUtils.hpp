#pragma once
#include <Geode/Prelude.hpp>

#include "Geode/utils/general.hpp"
#include "serialise/DeserialisationContext.h"

using namespace geode::prelude;

struct ByteWriter {
    ByteVector buffer;

    template <typename T>
    ByteWriter& operator<<(T const& value) {
        serialize(*this, value);
        return *this;
    }

private:
    template <typename T>
    friend void serialize(ByteWriter&, T const&);
};

template <typename T>
void serialize(ByteWriter& writer, T const& value) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    const auto offset = writer.buffer.size();
    writer.buffer.resize(writer.buffer.size() + sizeof(T));
    std::memcpy(writer.buffer.data() + offset, &value, sizeof(T));
}

struct ByteReader {
    ByteSpan buffer;
    size_t offset = 0;
    DeserialisationContext context;

    template <typename T>
    ByteReader& operator>>(T& value) {
        deserialize(*this, value);
        return *this;
    }

    bool hasRemaining() const {
        return offset < buffer.size();
    }

private:
    template <typename T>
    friend void deserialize(ByteReader&, T&);
};

template <typename T>
void deserialize(ByteReader& reader, T& value) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    if (reader.offset + sizeof(T) > reader.buffer.size()) {
        throw std::runtime_error("Not enough data to deserialize");
    }
    std::memcpy(&value, reader.buffer.data() + reader.offset, sizeof(T));
    reader.offset += sizeof(T);
}

struct ByteSize {
    uint64_t bytes;
};

template<>
struct fmt::formatter<ByteSize> {
    static constexpr auto parse(const format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const ByteSize bs, const format_context& ctx) {
        constexpr std::array<std::string_view, 6> units =
        {"B", "KB", "MB", "GB", "TB", "PB"};

        double size = bs.bytes;
        size_t index = 0;
        std::string formatString = "{:.0f} {}";

        while (size >= 1024.0 && index < units.size() - 1) {
            size /= 1024.0;
            index++;
            formatString[3] = '2';
        }

        return fmt::vformat_to(ctx.out(), formatString, fmt::make_format_args(size, units[index]));
    }
};