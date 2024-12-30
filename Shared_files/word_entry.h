#pragma once

#include <cstdint>
#include "project_types.h"

struct word_entry {
    id_type file_id;
    id_type position; // word number

    inline constexpr word_entry(id_type file_id = 0, id_type position = 0)
        : file_id(file_id), position(position) {}
};

inline constexpr bool operator==(const word_entry& left, const word_entry& right) {
    return (left.file_id == right.file_id && left.position == right.position);
}

inline constexpr bool operator<(const word_entry& left, const word_entry& right) {
    return (left.file_id < right.file_id ? true : (left.file_id == right.file_id && left.position < right.position));
}

namespace std {
    template <>
    struct hash<word_entry> {
        std::size_t operator()(const word_entry& entry) const noexcept {
            std::size_t h1 = std::hash<id_type>{}(entry.file_id);
            std::size_t h2 = std::hash<id_type>{}(entry.position);
            return h1 ^ (h2 << 1);
        }
    };
}
