#pragma once
#include <Geode/Prelude.hpp>

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

struct ByteReader {
    ByteSpan buffer;
    size_t offset = 0;

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
void serialize(ByteWriter& writer, T const& value) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    const auto offset = writer.buffer.size();
    writer.buffer.resize(writer.buffer.size() + sizeof(T));
    std::memcpy(writer.buffer.data() + offset, &value, sizeof(T));
}

template <typename T>
void deserialize(ByteReader& reader, T& value) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    if (reader.offset + sizeof(T) > reader.buffer.size()) {
        throw std::runtime_error("Not enough data to deserialize");
    }
    std::memcpy(&value, reader.buffer.data() + reader.offset, sizeof(T));
    reader.offset += sizeof(T);
}