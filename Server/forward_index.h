#pragma once

#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include "concurrent_utility.h"
#include "utility.h"
#include "project_types.h"
#include "word_entry.h"

class forward_index {
public:
    inline forward_index() = default;
    inline ~forward_index() { clear(); }

    inline forward_index(const forward_index& other) = delete;
    inline forward_index(forward_index&& other) = delete;
    inline forward_index& operator=(const forward_index& rhs) = delete;
    inline forward_index& operator=(forward_index&& rhs) = delete;

public:
    inline bool empty() const;
    inline bool empty_unsafe() const;

    inline std::size_t size() const;
    inline std::size_t size_unsafe() const;

    // Get the amount of word IDs for the specified file_id (the amount of unique words in the file)
    inline std::size_t size_word_id_set(id_type file_id) const;
    inline std::size_t size_word_id_set_unsafe(id_type file_id) const;

    inline void clear();
    inline void clear_unsafe();

    inline void add_word_id(id_type file_id, id_type word_id);
    inline void add_word_id_unsafe(id_type file_id, id_type word_id);

    inline void add_word_id_set(id_type file_id, const std::unordered_set<id_type>& word_ids);
    inline void add_word_id_set_unsafe(id_type file_id, const std::unordered_set<id_type>& word_ids);

    inline void add_word_id_set(id_type file_id, std::unordered_set<id_type>&& word_ids);
    inline void add_word_id_set_unsafe(id_type file_id, std::unordered_set<id_type>&& word_ids);

    // Completely delete the specified file_id with all the word IDs for it
    inline void delete_file(id_type file_id);
    inline void delete_file_unsafe(id_type file_id);

    // Only clear all the word IDs for the specified file_id, 
    // but do not actually erase the file_id completely along with destroying its set,
    // leaving file_id with an empty word ID set.
    inline void clear_file(id_type file_id);
    inline void clear_file_unsafe(id_type file_id);

    inline std::unordered_set<id_type> get_word_id_set(id_type file_id) const;
    inline std::unordered_set<id_type> get_word_id_set_unsafe(id_type file_id) const;

    inline const std::unordered_set<id_type>& get_word_id_set_ñref(id_type file_id) const;
    inline const std::unordered_set<id_type>& get_word_id_set_ñref_unsafe(id_type file_id) const;

    inline bool has_id(id_type file_id) const;
    inline bool has_id_unsafe(id_type file_id) const;

private:
    // key - file ID, value - un_set of word IDs
    using forward_map = std::unordered_map<id_type, std::unordered_set<id_type>>;

    mutable read_write_lock rw_lock;
    forward_map file_map;
};

// empty
inline bool forward_index::empty() const {
    read_lock r_lock(rw_lock);
    return empty_unsafe();
}

inline bool forward_index::empty_unsafe() const {
    return file_map.empty();
}

// size
inline std::size_t forward_index::size() const {
    read_lock r_lock(rw_lock);
    return size_unsafe();
}

inline std::size_t forward_index::size_unsafe() const {
    return file_map.size();
}

// size_word_id_set
inline std::size_t forward_index::size_word_id_set(id_type file_id) const {
    read_lock r_lock(rw_lock);
    return size_word_id_set_unsafe(file_id);
}

inline std::size_t forward_index::size_word_id_set_unsafe(id_type file_id) const {
    auto it = file_map.find(file_id);
    if (it == file_map.end()) {
        throw std::out_of_range("File ID not found.");
    }
    return it->second.size();
}

// clear
inline void forward_index::clear() {
    write_lock w_lock(rw_lock);
    clear_unsafe();
}

inline void forward_index::clear_unsafe() {
    file_map.clear();
}

// add_word_id
inline void forward_index::add_word_id(id_type file_id, id_type word_id) {
    write_lock w_lock(rw_lock);
    add_word_id_unsafe(file_id, word_id);
}

inline void forward_index::add_word_id_unsafe(id_type file_id, id_type word_id) {
    file_map[file_id].insert(word_id);
}

// add_word_id_set
inline void forward_index::add_word_id_set(id_type file_id, const std::unordered_set<id_type>& word_ids) {
    write_lock w_lock(rw_lock);
    add_word_id_set_unsafe(file_id, word_ids);
}

inline void forward_index::add_word_id_set_unsafe(id_type file_id, const std::unordered_set<id_type>& word_ids) {
    file_map[file_id].insert(std::begin(word_ids), std::end(word_ids));
}

inline void forward_index::add_word_id_set(id_type file_id, std::unordered_set<id_type>&& word_ids) {
    write_lock w_lock(rw_lock);
    add_word_id_set_unsafe(file_id, std::move(word_ids));
}

inline void forward_index::add_word_id_set_unsafe(id_type file_id, std::unordered_set<id_type>&& word_ids) {
    file_map[file_id].merge(word_ids);
}

// delete_file
inline void forward_index::delete_file(id_type file_id) {
    write_lock w_lock(rw_lock);
    delete_file_unsafe(file_id);
}

inline void forward_index::delete_file_unsafe(id_type file_id) {
    auto it = file_map.find(file_id);
    if (it == file_map.end()) {
        throw std::out_of_range("File ID not found.");
    }

    file_map.erase(it);
}

// clear_file
inline void forward_index::clear_file(id_type file_id) {
    write_lock w_lock(rw_lock);
    clear_file_unsafe(file_id);
}

inline void forward_index::clear_file_unsafe(id_type file_id) {
    auto it = file_map.find(file_id);
    if (it == file_map.end()) {
        throw std::out_of_range("File ID not found.");
    }

    file_map[file_id].clear();
}

// get_word_id_set
inline std::unordered_set<id_type> forward_index::get_word_id_set(id_type file_id) const {
    return get_word_id_set_ñref(file_id);
}

inline std::unordered_set<id_type> forward_index::get_word_id_set_unsafe(id_type file_id) const {
    return get_word_id_set_ñref_unsafe(file_id);
}

inline const std::unordered_set<id_type>& forward_index::get_word_id_set_ñref(id_type file_id) const {
    read_lock r_lock(rw_lock);
    return get_word_id_set_ñref_unsafe(file_id);
}

inline const std::unordered_set<id_type>& forward_index::get_word_id_set_ñref_unsafe(id_type file_id) const {
    auto it = file_map.find(file_id);
    if (it == file_map.end()) {
        throw std::out_of_range("File ID not found.");
    }
    return it->second;
}

// has_id
inline bool forward_index::has_id(id_type file_id) const {
    read_lock r_lock(rw_lock);
    return has_id_unsafe(file_id);
}

inline bool forward_index::has_id_unsafe(id_type file_id) const {
    return file_map.find(file_id) != file_map.end();
}
