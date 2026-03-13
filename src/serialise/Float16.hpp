#pragma once
#include <util/ByteUtils.hpp>

struct Float16 {
    uint16_t bits;

    static Float16 fromFloat(const float value) {
        if (std::isnan(value)) {
            return {0x7E00}; // NaN
        }
        if (std::isinf(value)) {
            return {static_cast<uint16_t>(value < 0 ? 0xFC00u : 0x7C00u)}; // ±inf
        }

        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(float));

        const uint16_t sign = (bits >> 31) << 15;
        const uint16_t exponent = (bits >> 23 & 0xFF) - 112;
        const uint16_t mantissa = bits >> 13 & 0x3FF;

        if (exponent <= 0) {
            return {sign}; // underflow to zero
        }
        if (exponent >= 31) {
            return {static_cast<uint16_t>(sign | 0x7C00)}; // overflow to inf
        }

        Float16 result{};
        result.bits = sign | exponent << 10 | mantissa;
        return result;
    }

    [[nodiscard]] float toFloat() const {
        const uint16_t sign = bits >> 15 & 1;
        const uint16_t exponent = bits >> 10 & 0x1F;
        const uint16_t mantissa = bits & 0x3FF;

        if (exponent == 0) {
            return sign ? -0.0f : 0.0f;
        }

        const uint32_t sign32 = sign << 31;
        const uint32_t exponent32 = (exponent + 112) << 23;
        const uint32_t mantissa32 = mantissa << 13;

        const uint32_t value32 = sign32 | exponent32 | mantissa32;
        float result;
        std::memcpy(&result, &value32, sizeof(float));
        return result;
    }
};

inline void serialize(ByteWriter& writer, const Float16& float16) {
    writer << float16.bits;
}

inline void deserialize_v1(ByteReader& reader, Float16& float16) {
    reader >> float16.bits;
}