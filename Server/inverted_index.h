#pragma once

#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include "concurrent_utility.h"
#include "utility.h"
#include "project_types.h"
#include "word_entry.h"

class inverted_index {
public:
    inline inverted_index() = default;
    inline ~inverted_index() { clear(); }

    inline inverted_index(const inverted_index& other) = delete;
    inline inverted_index(inverted_index&& other) = delete;
    inline inverted_index& operator=(const inverted_index& rhs) = delete;
    inline inverted_index& operator=(inverted_index&& rhs) = delete;

public:
    inline bool empty() const;
    inline bool empty_unsafe() const;

    inline std::size_t size() const;
    inline std::size_t size_unsafe() const;

    // Returns true if there's an empty existing set of word entries for the specified word_id
    inline bool empty_word_entry_set(id_type word_id) const;
    inline bool empty_word_entry_set_unsafe(id_type word_id) const;

    // Returns true if there's an empty existing set of file IDs for the specified word_id
    inline bool empty_file_set(id_type word_id) const;
    inline bool empty_file_set_unsafe(id_type word_id) const;

    // Get the amount of word entries for the specified word_id
    inline std::size_t size_word_entry_set(id_type word_id) const;
    inline std::size_t size_word_entry_set_unsafe(id_type word_id) const;

    // Get the amount of file IDs for the specified word_id
    inline std::size_t size_file_set(id_type word_id) const;
    inline std::size_t size_file_set_unsafe(id_type word_id) const;

    inline void clear();
    inline void clear_unsafe();

    inline void add_word_entry(id_type word_id, const word_entry& single_word_entry);
    inline void add_word_entry_unsafe(id_type word_id, const word_entry& single_word_entry);

    inline void add_word_entry_set(id_type word_id, const std::unordered_set<word_entry>& word_entries);
    inline void add_word_entry_set_unsafe(id_type word_id, const std::unordered_set<word_entry>& word_entries);
    
    inline void add_word_entry_set(id_type word_id, std::unordered_set<word_entry>&& word_entries);
    inline void add_word_entry_set_unsafe(id_type word_id, std::unordered_set<word_entry>&& word_entries);

    // Delete all word entries for word_id with only the specified file_id, remaining word entries with other file IDs.
    // Does NOT erases the set itself, even if it becomes empty
    inline void clear_for_word_and_file(id_type word_id, id_type file_id);
    inline void clear_for_word_and_file_unsafe(id_type word_id, id_type file_id);

    inline std::unordered_set<word_entry> get_word_entry_set(id_type word_id) const;
    inline std::unordered_set<word_entry> get_word_entry_set_unsafe(id_type word_id) const;
    inline const std::unordered_set<word_entry>& get_word_entry_set_cref(id_type word_id) const;
    inline const std::unordered_set<word_entry>& get_word_entry_set_cref_unsafe(id_type word_id) const;
    inline const std::unordered_set<word_entry>* get_word_entry_set_cp(id_type word_id) const;
    inline const std::unordered_set<word_entry>* get_word_entry_set_cp_unsafe(id_type word_id) const;

    inline std::unordered_set<id_type> get_file_set(id_type word_id) const;
    inline std::unordered_set<id_type> get_file_set_unsafe(id_type word_id) const;
    inline const std::unordered_set<id_type>& get_file_set_cref(id_type word_id) const;
    inline const std::unordered_set<id_type>& get_file_set_cref_unsafe(id_type word_id) const;
    inline const std::unordered_set<id_type>* get_file_set_cp(id_type word_id) const;
    inline const std::unordered_set<id_type>* get_file_set_cp_unsafe(id_type word_id) const;

    inline bool has_id(id_type word_id) const;
    inline bool has_id_unsafe(id_type word_id) const;

private:
    // key - word ID, value - un_set of word entries
    using inverted_map_entries = std::unordered_map<id_type, std::unordered_set<word_entry>>;
    // key - word ID, value - un_set of file IDs
    using inverted_map = std::unordered_map<id_type, std::unordered_set<id_type>>;

    mutable read_write_lock rw_lock;
    inverted_map_entries word_entries_map;
    inverted_map word_map;
};

// empty
inline bool inverted_index::empty() const {
    read_lock r_lock(rw_lock);
    return empty_unsafe();
}

inline bool inverted_index::empty_unsafe() const {
    return word_map.empty(); // same as word_entries_map.empty()
}

// size
inline std::size_t inverted_index::size() const {
    read_lock r_lock(rw_lock);
    return size_unsafe();
}

inline std::size_t inverted_index::size_unsafe() const {
    return word_map.size(); // same as word_entries_map.size()
}

// empty_word_set
inline bool inverted_index::empty_word_entry_set(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return empty_word_entry_set_unsafe(word_id);
}

inline bool inverted_index::empty_word_entry_set_unsafe(id_type word_id) const {
    auto it = word_entries_map.find(word_id);
    if (it == word_entries_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second.empty();
}

// empty_file_set
inline bool inverted_index::empty_file_set(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return empty_file_set_unsafe(word_id);
}

inline bool inverted_index::empty_file_set_unsafe(id_type word_id) const {
    auto it = word_map.find(word_id);
    if (it == word_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second.empty();
}

// size_word_set
inline std::size_t inverted_index::size_word_entry_set(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return size_word_entry_set_unsafe(word_id);
}

inline std::size_t inverted_index::size_word_entry_set_unsafe(id_type word_id) const {
    auto it = word_entries_map.find(word_id);
    if (it == word_entries_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second.size();
}

// size_file_set
inline std::size_t inverted_index::size_file_set(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return size_file_set_unsafe(word_id);
}

inline std::size_t inverted_index::size_file_set_unsafe(id_type word_id) const {
    auto it = word_map.find(word_id);
    if (it == word_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second.size();
}

// clear
inline void inverted_index::clear() {
    write_lock w_lock(rw_lock);
    clear_unsafe();
}

inline void inverted_index::clear_unsafe() {
    word_entries_map.clear();
    word_map.clear();
}

// add_word_entry
inline void inverted_index::add_word_entry(id_type word_id, const word_entry& single_word_entry) {
    write_lock w_lock(rw_lock);
    add_word_entry_unsafe(word_id, single_word_entry);
}

inline void inverted_index::add_word_entry_unsafe(id_type word_id, const word_entry& single_word_entry) {
    word_entries_map[word_id].insert(single_word_entry);
    word_map[word_id].insert(single_word_entry.file_id);
}

// add_word_entry_set
inline void inverted_index::add_word_entry_set(id_type word_id, const std::unordered_set<word_entry>& word_entries) {
    write_lock w_lock(rw_lock);
    add_word_entry_set_unsafe(word_id, word_entries);
}

inline void inverted_index::add_word_entry_set_unsafe(id_type word_id, const std::unordered_set<word_entry>& word_entries) {
    for (const auto& entry : word_entries) {
        word_map[word_id].insert(entry.file_id);
    }
    word_entries_map[word_id].insert(std::begin(word_entries), std::end(word_entries));
}

inline void inverted_index::add_word_entry_set(id_type word_id, std::unordered_set<word_entry>&& word_entries) {
    write_lock w_lock(rw_lock);
    add_word_entry_set_unsafe(word_id, std::move(word_entries));
}

inline void inverted_index::add_word_entry_set_unsafe(id_type word_id, std::unordered_set<word_entry>&& word_entries) {
    for (const auto& entry : word_entries) {
        word_map[word_id].insert(entry.file_id);
    }
    word_entries_map[word_id].merge(word_entries);
}

// clear_for_word_and_file
inline void inverted_index::clear_for_word_and_file(id_type word_id, id_type file_id) {
    write_lock w_lock(rw_lock);
    clear_for_word_and_file_unsafe(word_id, file_id);
}

inline void inverted_index::clear_for_word_and_file_unsafe(id_type word_id, id_type file_id) {
    auto it = word_entries_map.find(word_id);
    if (it == word_entries_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }

    auto& entry_set = it->second;
    std::erase_if(entry_set, [file_id](const word_entry& entry) {
        return entry.file_id == file_id;
    });

    word_map[word_id].erase(file_id);
}

// get_word_entry_set
inline std::unordered_set<word_entry> inverted_index::get_word_entry_set(id_type word_id) const {
    return get_word_entry_set_cref(word_id);
}

inline std::unordered_set<word_entry> inverted_index::get_word_entry_set_unsafe(id_type word_id) const {
    return get_word_entry_set_cref_unsafe(word_id);
}

inline const std::unordered_set<word_entry>& inverted_index::get_word_entry_set_cref(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return get_word_entry_set_cref_unsafe(word_id);
}

inline const std::unordered_set<word_entry>& inverted_index::get_word_entry_set_cref_unsafe(id_type word_id) const {
    auto it = word_entries_map.find(word_id);
    if (it == word_entries_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second;
}

inline const std::unordered_set<word_entry>* inverted_index::get_word_entry_set_cp(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return get_word_entry_set_cp_unsafe(word_id);
}

inline const std::unordered_set<word_entry>* inverted_index::get_word_entry_set_cp_unsafe(id_type word_id) const {
    auto it = word_entries_map.find(word_id);
    if (it == word_entries_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return &it->second;
}

// get_file_set
inline std::unordered_set<id_type> inverted_index::get_file_set(id_type word_id) const {
    return get_file_set_cref(word_id);
}

inline std::unordered_set<id_type> inverted_index::get_file_set_unsafe(id_type word_id) const {
    return get_file_set_cref_unsafe(word_id);
}

inline const std::unordered_set<id_type>& inverted_index::get_file_set_cref(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return get_file_set_cref_unsafe(word_id);
}

inline const std::unordered_set<id_type>& inverted_index::get_file_set_cref_unsafe(id_type word_id) const {
    auto it = word_map.find(word_id);
    if (it == word_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return it->second;
}

inline const std::unordered_set<id_type>* inverted_index::get_file_set_cp(id_type word_id) const {
    read_lock r_lock(rw_lock);
    return get_file_set_cp_unsafe(word_id);
}

inline const std::unordered_set<id_type>* inverted_index::get_file_set_cp_unsafe(id_type word_id) const {
    auto it = word_map.find(word_id);
    if (it == word_map.end()) {
        throw std::out_of_range("Word ID not found.");
    }
    return &it->second;
}

// has_id
inline bool inverted_index::has_id(id_type word_id) const {
    read_lock r_lock(rw_lock);
    has_id_unsafe(word_id);
}

inline bool inverted_index::has_id_unsafe(id_type word_id) const {
    return word_map.find(word_id) != word_map.end();
}
