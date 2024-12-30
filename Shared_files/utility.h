#pragma once

#include <bit>
#include <cstdint>
#include <string>
#include <codecvt>
//#include <cwctype>
#include "project_types.h"

// ===============================
// Utilities for normalizing text:
// ===============================
template <typename char_type>
class text_normalizer {
public:
    static inline void to_upper(std::basic_string<char_type>& str) {
        get_ctype().toupper(&str[0], &str[0] + str.size());
    }

    static inline void to_lower(std::basic_string<char_type>& str) {
        get_ctype().tolower(&str[0], &str[0] + str.size());
    }

    struct normalizer_whitespace : std::ctype<char_type> {
        bool do_is(std::ctype_base::mask m, std::ctype<char_type>::char_type c) const override
        {
            if (m & std::ctype_base::space) {
                return !std::ctype<char_type>::do_is(std::ctype_base::alnum, c); // Everything that is not alnum will be classified as whitespace
            }

            return std::ctype<char_type>::do_is(m, c); // Leave the rest to the base class
        }
    };

    static inline const std::locale& get_locale() {
        static const std::locale loc(""); // Change to lambda-statement that will encapsulate setting global locale?
        return loc;
    }

    static inline std::locale new_ss_locale() {
        normalizer_whitespace* ss_ws = new normalizer_whitespace;
        return std::locale(get_locale(), ss_ws);
    }

    static inline const std::ctype<char_type>& get_ctype() {
        static const std::ctype<char_type>& ctype = std::use_facet<std::ctype<char_type>>(get_locale());
        return ctype;
    }

    static inline const std::ctype<char_type>& get_ss_ctype() {
        static const std::ctype<char_type>& ctype = std::use_facet<normalizer_whitespace>(get_locale());
        return ctype;
    }
};

// ======================================================================
// Utilities for converting data to bytes for the network and vice versa:
// ======================================================================
template <typename char_type>
class utf_converter {
public:
    static inline std::basic_string<char_type> utf8_to_string_type(const std::string& utf8_str) {
        return converter.from_bytes(utf8_str);
    }

    static inline std::string string_type_to_utf8(const std::basic_string<char_type>& multibyte_str) {
        return converter.to_bytes(multibyte_str);
    }

private:
    static inline thread_local std::wstring_convert<std::codecvt_utf8<char_type>, char_type> converter;
};

// Write big-endian bytes into an integer of type T
template <typename T>
inline T from_big_endian(const char* buffer) {
    static_assert(std::is_integral_v<T>, "Only integral types are supported.");

    T value;
    std::memcpy(&value, buffer, sizeof(value));

    if constexpr (std::endian::native != std::endian::big) {
        value = std::byteswap(value);
    }

    return value;
}

// Write an integer of type T into big-endian bytes
template <typename T>
inline void to_big_endian(T value, char* buffer) {
    static_assert(std::is_integral_v<T>, "Only integral types are supported.");

    if constexpr (std::endian::native != std::endian::big) {
        value = std::byteswap(value);
    }

    std::memcpy(buffer, &value, sizeof(value));
}

// Convert 4-byte integer of specified type to IEEE-754 float
template <typename T = std::uint32_t>
inline float integer_to_ieee754(T value) {
    static_assert(sizeof(T) == 4, "Only 4-byte integral types are supported.");
    static_assert(sizeof(float) == 4, "Float is not in IEEE-754 format on this machine.");

    float binary;
    std::memcpy(&binary, &value, sizeof(value));

    return binary;
}

// Convert IEEE-754 float to 4-byte integer of specified type
template <typename T = std::uint32_t>
inline T ieee754_to_integer(float value) {
    static_assert(sizeof(T) == 4, "Only 4-byte integral types are supported.");
    static_assert(sizeof(float) == 4, "Float is not in IEEE-754 format on this machine.");

    T binary;
    std::memcpy(&binary, &value, sizeof(value));

    return binary;
}
