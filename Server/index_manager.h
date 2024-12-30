#pragma once

#include <fstream>
#include <filesystem>
#include "inverted_index.h"
#include "forward_index.h"
#include "id_value_table.h"
#include "concurrent_utility.h"
#include "utility.h"
#include "project_types.h"
#include "word_entry.h"

template <typename string_type>
class index_manager {
public:
    inline index_manager() = default;
    inline ~index_manager() = default;

    inline index_manager(const index_manager& other) = delete;
    inline index_manager(index_manager&& other) = delete;
    inline index_manager& operator=(const index_manager& rhs) = delete;
    inline index_manager& operator=(index_manager&& rhs) = delete;

public:
    // Returns pair where .first is true if the file is actually present, false otherwise
    inline std::pair<bool, id_type> has_file(const string_type& file_path);
    inline std::pair<bool, id_type> has_file(string_type&& file_path);

    // Add method for getting ALL the files?

    inline bool get_file_content_utf8(const string_type& file_path, std::string& out_file_content);

    inline bool add_file(const string_type& file_path);
    inline bool add_file(string_type&& file_path);
    inline bool add_create_file(const string_type& file_path, const string_type& file_content);
    inline bool add_create_file(string_type&& file_path, string_type&& file_content);

    inline bool remove_file(const string_type& file_path);
    inline bool remove_file(string_type&& file_path);

    inline bool modify_file(const string_type& file_path);
    inline bool modify_file(string_type&& file_path);

    inline void clear_all();

    inline std::pair<bool, id_type> get_word_entry_set_for_word(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_word_entry_set_for_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_word_entry_set_for_lowered_word(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_word_entry_set_for_lowered_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;

    inline bool get_word_entry_set_for_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<word_entry>& out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline bool get_word_entry_set_for_lowered_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<word_entry>& out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;

    inline std::pair<bool, id_type> get_file_set_for_word(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_file_set_for_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_file_set_for_lowered_word(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> get_file_set_for_lowered_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;

    inline bool get_file_set_for_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<id_type>& out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline bool get_file_set_for_lowered_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<id_type>& out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;

private:
    inline std::pair<bool, id_type> do_has_file(string_type&& file_path);
    inline std::pair<bool, id_type> do_has_file_lowered(string_type&& file_path);
    inline std::pair<bool, id_type> do_has_file_lowered_unsafe(string_type&& file_path);

    inline bool do_add_file(string_type&& file_path);
    inline bool do_add_create_file(string_type&& file_path, string_type&& file_content);
    inline bool add_words_from_file_to_index(std::vector<string_type>&& words, id_type file_id, string_type&& file_path);

    inline bool do_remove_file(string_type&& file_path);
    inline bool do_modify_file(string_type&& file_path);

    inline std::pair<bool, id_type> do_get_word_entry_set_for_lowered_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const;
    inline std::pair<bool, id_type> do_get_file_set_for_lowered_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const;

    inline std::pair<bool, id_type> get_word_entry_set_for_word_unsafe(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries) const;
    inline std::pair<bool, id_type> get_file_set_for_word_unsafe(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids) const;

    inline string_type read_file(const string_type& file_path) const;
    inline std::string read_file_as_utf8(const string_type& file_path) const;
    inline std::vector<string_type> parse_and_normalize_words(string_type&& content) const;
    inline std::vector<string_type> parse_and_normalize_words_ss(string_type&& content) const; // using stringstream (x1.5-3 times slower)

private:
    using char_type = string_type::value_type;
    using string_table = id_value_table<id_type, string_type>;
    using presence_table = id_value_table<id_type, bool, false>;

    mutable read_write_lock rw_lock;

    inverted_index inverted;
    forward_index forward;

    string_table words_table;
    string_table files_table;
    presence_table files_present_table;
};

// has_file
template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::has_file(const string_type& file_path) {
    string_type to_lower_file_path(file_path);
    return do_has_file(std::move(to_lower_file_path));
}

template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::has_file(string_type&& file_path) {
    return do_has_file(std::move(file_path));
}

template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::do_has_file(string_type&& file_path) {
    text_normalizer<char_type>::to_lower(file_path);
    return do_has_file_lowered(std::move(file_path));
}

template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::do_has_file_lowered(string_type&& file_path) {
    id_type file_id = 0;
    bool file_present = false;
    {
        read_lock r_lock(rw_lock);

        file_id = files_table.get_value_id_always_unsafe(file_path);
        file_present = file_id != 0 ? files_present_table.get_value_unsafe(file_id) : false;
    }

    return { file_present, file_id };
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::do_has_file_lowered_unsafe(string_type&& file_path) {
    id_type file_id = files_table.get_value_id_always_unsafe(file_path);
    bool file_present = file_id != 0 ? files_present_table.get_value_unsafe(file_id) : false;

    return { file_present, file_id };
}

// get_file_content_utf8
template<typename string_type>
inline bool index_manager<string_type>::get_file_content_utf8(const string_type& file_path, std::string& out_file_content) {
    // Somewhere exceptions are thrown, somewhere just bool is returned as a result. This inconsistency is bad, remove exceptions later.
    try {
        out_file_content = read_file_as_utf8(file_path);
        return true;
    }
    catch(std::exception&) {
        return false;
    }
}

// add_file
template <typename string_type>
inline bool index_manager<string_type>::add_file(const string_type& file_path) {
    string_type to_lower_file_path(file_path);
    return do_add_file(std::move(to_lower_file_path));
}

template <typename string_type>
inline bool index_manager<string_type>::add_file(string_type&& file_path) {
    return do_add_file(std::move(file_path));
}

template<typename string_type>
inline bool index_manager<string_type>::add_create_file(const string_type& file_path, const string_type& file_content) {
    string_type to_lower_file_path(file_path);
    string_type to_lower_file_content(file_content);
    return do_add_create_file(std::move(to_lower_file_path), std::move(to_lower_file_content));
}

template<typename string_type>
inline bool index_manager<string_type>::add_create_file(string_type&& file_path, string_type&& file_content) {
    return do_add_create_file(std::move(file_path), std::move(file_content));
}

template <typename string_type>
inline bool index_manager<string_type>::do_add_file(string_type&& file_path) {
    text_normalizer<char_type>::to_lower(file_path);
    auto file_found = do_has_file_lowered(std::move(file_path)); // Does not actually move - not a bag, file_path remains valid
    if (file_found.first == true) {
        return false;
    }

    string_type file_content;
    try {
        file_content = read_file(file_path);
    }
    catch(std::exception&) {
        return false;
    }

    std::vector<string_type> words = parse_and_normalize_words(std::move(file_content));

    id_type file_id = file_found.second;
    return add_words_from_file_to_index(std::move(words), file_id, std::move(file_path));
}

template<typename string_type>
inline bool index_manager<string_type>::do_add_create_file(string_type&& file_path, string_type&& file_content) {
    text_normalizer<char_type>::to_lower(file_path);
    auto file_found = do_has_file_lowered(std::move(file_path));
    if (file_found.first == true) {
        return false;
    }

    // Create the file and write into it
    std::filesystem::path file_path_actual(file_path);
    if (std::filesystem::exists(file_path_actual)) {
        return false;
    }

    if (file_path_actual.has_parent_path()) {
        std::filesystem::create_directories(file_path_actual.parent_path());
    }

    std::ofstream file;

    if constexpr (std::is_same<char_type, char16_t>::value || std::is_same<char_type, char32_t>::value) {
        std::filesystem::path converted_file_path(file_path);
        file.open(converted_file_path.generic_wstring(), std::ios::out | std::ios::binary);
    }
    else {
        file.open(file_path, std::ios::out | std::ios::binary);
    }

    if (!file) {
        return false;
    }

    if constexpr (std::is_same_v<string_type, std::string>) {
        file << file_content;
    }
    else if constexpr (std::is_same_v<string_type, std::u8string>) {
        std::string utf8_file_content(reinterpret_cast<const char*>(file_content.data()), file_content.size());
        file << utf8_file_content;
    }
    else {
        std::string utf8_file_content = utf_converter<char_type>::string_type_to_utf8(file_content);
        file << utf8_file_content;
    }
    file.close();

    std::vector<string_type> words = parse_and_normalize_words(std::move(file_content));

    id_type file_id = file_found.second;
    return add_words_from_file_to_index(std::move(words), file_id, std::move(file_path));
}

template<typename string_type>
inline bool index_manager<string_type>::add_words_from_file_to_index(std::vector<string_type>&& words, id_type file_id, string_type&& file_path) {
    std::unordered_set<id_type> word_ids;
    word_ids.reserve(words.size());

    write_lock w_lock(rw_lock);

    if (file_id == 0) {
        file_id = files_table.add_value_unsafe(std::move(file_path));
        files_present_table.add_value_unsafe(true);
    }
    else {
        files_present_table.modify_by_id_unsafe(file_id, true);
    }

    id_type position = 1;
    for (auto& word : words) {
        id_type word_id = words_table.get_value_id_always_unsafe(word);
        if (word_id == 0) { word_id = words_table.add_value_unsafe(std::move(word)); }

        inverted.add_word_entry_unsafe(word_id, word_entry(file_id, position++));
        word_ids.insert(word_id);
    }
    forward.add_word_id_set_unsafe(file_id, std::move(word_ids));

    return true;
}

// remove_file
template <typename string_type>
inline bool index_manager<string_type>::remove_file(const string_type& file_path) {
    string_type to_lower_file_path(file_path);
    return do_remove_file(std::move(to_lower_file_path));
}

template <typename string_type>
inline bool index_manager<string_type>::remove_file(string_type&& file_path) {
    return do_remove_file(std::move(file_path));
}

template <typename string_type>
inline bool index_manager<string_type>::do_remove_file(string_type&& file_path) {
    text_normalizer<char_type>::to_lower(file_path);

    auto file_found = do_has_file_lowered(std::move(file_path));
    if (file_found.first == false) {
        return false;
    }

    write_lock w_lock(rw_lock);

    id_type file_id = file_found.second;
    
    const auto& del_word_ids = forward.get_word_id_set_ñref_unsafe(file_id);
    for (const auto& del_word_id : del_word_ids) {
        inverted.clear_for_word_and_file_unsafe(del_word_id, file_id);
    }
    forward.clear_file_unsafe(file_id);

    files_present_table.modify_by_id_unsafe(file_id, false);

    return true;
}

// modify_file
template <typename string_type>
inline bool index_manager<string_type>::modify_file(const string_type& file_path) {
    string_type to_lower_file_path(file_path);
    return do_modify_file(std::move(to_lower_file_path));
}

template <typename string_type>
inline bool index_manager<string_type>::modify_file(string_type&& file_path) {
    return do_modify_file(std::move(file_path));
}

template <typename string_type>
inline bool index_manager<string_type>::do_modify_file(string_type&& file_path) {
    text_normalizer<char_type>::to_lower(file_path);
    auto file_found = do_has_file_lowered(std::move(file_path));
    if (file_found.first == false) {
        return false;
    }

    id_type file_id = file_found.second;

    std::vector<string_type> words = parse_and_normalize_words(read_file(file_path));
    std::unordered_set<id_type> word_ids;
    word_ids.reserve(words.size());

    write_lock w_lock(rw_lock);

    const auto& del_word_ids = forward.get_word_id_set_ñref_unsafe(file_id);
    for (const auto& del_word_id : del_word_ids) {
        inverted.clear_for_word_and_file_unsafe(del_word_id, file_id);
    }
    forward.clear_file_unsafe(file_id);

    id_type position = 1;
    for (const auto& word : words) {
        id_type word_id = words_table.get_value_id_always_unsafe(word);
        if (word_id == 0) { word_id = words_table.add_value_unsafe(word); }

        inverted.add_word_entry_unsafe(word_id, word_entry(file_id, position++));
        word_ids.insert(word_id);
    }
    forward.add_word_id_set_unsafe(file_id, std::move(word_ids));

    return true;
}

// clear_all
template <typename string_type>
inline void index_manager<string_type>::clear_all() {
    write_lock w_lock(rw_lock);

    inverted.clear_unsafe();
    forward.clear_unsafe();
    words_table.clear_unsafe();
    files_table.clear_unsafe();
    files_present_table.clear_unsafe();
}

// get_word_entry_set_for_word
template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_word_entry_set_for_word(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    string_type to_lower_word(word);
    text_normalizer<char_type>::to_lower(to_lower_word);
    return do_get_word_entry_set_for_lowered_word(std::move(to_lower_word), cp_out_word_entries, out_files_table);
}

template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_word_entry_set_for_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    text_normalizer<char_type>::to_lower(word);
    return do_get_word_entry_set_for_lowered_word(std::move(word), cp_out_word_entries, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_word_entry_set_for_lowered_word(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    string_type lowered_word(word);
    return do_get_word_entry_set_for_lowered_word(std::move(lowered_word), cp_out_word_entries, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_word_entry_set_for_lowered_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    return do_get_word_entry_set_for_lowered_word(std::move(word), cp_out_word_entries, out_files_table);
}

template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::do_get_word_entry_set_for_lowered_word(string_type&& word, const std::unordered_set<word_entry>*& cp_out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    read_lock r_lock(rw_lock);

    id_type word_id = words_table.get_value_id_always_unsafe(word);
    if (word_id == 0) {
        return { false, word_id };
    }

    cp_out_word_entries = inverted.get_word_entry_set_cp_unsafe(word_id);

    const auto& file_set_cref = inverted.get_file_set_cref_unsafe(word_id);
    out_files_table.reserve(file_set_cref.size());

    for (const auto file_id : file_set_cref) {
        out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
    }

    return { !cp_out_word_entries->empty(), word_id};
}

// get_word_entry_set_for_word_unsafe
template <typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_word_entry_set_for_word_unsafe(const string_type& word, const std::unordered_set<word_entry>*& cp_out_word_entries) const {
    id_type word_id = words_table.get_value_id_always_unsafe(word);
    if (word_id == 0) {
        return { false, word_id };
    }

    cp_out_word_entries = inverted.get_word_entry_set_cp_unsafe(word_id);
    return { !cp_out_word_entries->empty(), word_id };
}

// get_word_entry_set_for_word_set
template <typename string_type>
inline bool index_manager<string_type>::get_word_entry_set_for_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<word_entry>& out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    if (word_set.empty()) {
        return false;
    }

    std::unordered_map<id_type, std::unordered_set<word_entry>> file_entries_map;
    const std::unordered_set<word_entry>* p_word_entries;
    std::unordered_map<id_type, std::size_t> file_words_map;
    const std::unordered_set<id_type>* p_files;

    std::unordered_set<string_type> lowered_word_set;
    lowered_word_set.reserve(word_set.size());

    for (auto& word : word_set) {
        string_type lowered_word(word);
        text_normalizer<char_type>::to_lower(lowered_word);
        lowered_word_set.emplace(std::move(lowered_word));
    }

    read_lock r_lock(rw_lock);

    for (auto& word : lowered_word_set) {
        std::pair<bool, id_type> word_result_files = get_file_set_for_word_unsafe(word, p_files);
        std::pair<bool, id_type> word_result = get_word_entry_set_for_word_unsafe(word, p_word_entries);
        
        if (word_result.first == false) {
            return false; // If at least one word has no occurrences, the intersection is empty.
        }
        for (const auto& entry : *p_word_entries) {
            file_entries_map[entry.file_id].emplace(entry);
        }
        for (const auto file_id : *p_files) {
            ++file_words_map[file_id];
        }
    }

    std::size_t expected_count = lowered_word_set.size();
    for (const auto& [file_id, words_count] : file_words_map) {
        if (words_count == expected_count) {
            auto& entries = file_entries_map[file_id];
            out_word_entries.merge(std::move(entries));

            out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
        }
    }

    return !out_word_entries.empty();
}

// get_word_entry_set_for_lowered_word_set
template <typename string_type>
inline bool index_manager<string_type>::get_word_entry_set_for_lowered_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<word_entry>& out_word_entries, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    if (word_set.empty()) {
        return false;
    }

    std::unordered_map<id_type, std::unordered_set<word_entry>> file_entries_map;
    const std::unordered_set<word_entry>* p_word_entries;
    std::unordered_map<id_type, std::size_t> file_words_map;
    const std::unordered_set<id_type>* p_files;

    read_lock r_lock(rw_lock);

    for (auto& word : word_set) {
        std::pair<bool, id_type> word_result_files = get_file_set_for_word_unsafe(word, p_files);
        std::pair<bool, id_type> word_result = get_word_entry_set_for_word_unsafe(word, p_word_entries);

        if (word_result.first == false) {
            return false; // If at least one word has no occurrences, the intersection is empty.
        }
        for (const auto& entry : *p_word_entries) {
            file_entries_map[entry.file_id].emplace(entry);
        }
        for (const auto file_id : *p_files) {
            ++file_words_map[file_id];
        }
    }

    std::size_t expected_count = word_set.size();
    for (const auto& [file_id, words_count] : file_words_map) {
        if (words_count == expected_count) {
            auto& entries = file_entries_map[file_id];
            out_word_entries.merge(std::move(entries));

            out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
        }
    }

    return !out_word_entries.empty();
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_file_set_for_word(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    string_type to_lower_word(word);
    text_normalizer<char_type>::to_lower(to_lower_word);
    return do_get_file_set_for_lowered_word(std::move(to_lower_word), cp_out_file_ids, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_file_set_for_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    text_normalizer<char_type>::to_lower(word);
    return do_get_file_set_for_lowered_word(std::move(word), cp_out_file_ids, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_file_set_for_lowered_word(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    string_type lowered_word(word);
    return do_get_file_set_for_lowered_word(std::move(lowered_word), cp_out_file_ids, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_file_set_for_lowered_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    return do_get_file_set_for_lowered_word(std::move(word), cp_out_file_ids, out_files_table);
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::do_get_file_set_for_lowered_word(string_type&& word, const std::unordered_set<id_type>*& cp_out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    read_lock r_lock(rw_lock);

    id_type word_id = words_table.get_value_id_always_unsafe(word);
    if (word_id == 0) {
        return { false, word_id };
    }

    cp_out_file_ids = inverted.get_file_set_cp_unsafe(word_id);
    out_files_table.reserve(cp_out_file_ids->size());

    for (const auto file_id : *cp_out_file_ids) {
        out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
    }

    return { !cp_out_file_ids->empty(), word_id };
}

template<typename string_type>
inline std::pair<bool, id_type> index_manager<string_type>::get_file_set_for_word_unsafe(const string_type& word, const std::unordered_set<id_type>*& cp_out_file_ids) const {
    id_type word_id = words_table.get_value_id_always_unsafe(word);
    if (word_id == 0) {
        return { false, word_id };
    }

    cp_out_file_ids = inverted.get_file_set_cp_unsafe(word_id);
    return { !cp_out_file_ids->empty(), word_id };
}

template<typename string_type>
inline bool index_manager<string_type>::get_file_set_for_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<id_type>& out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    if (word_set.empty()) {
        return false;
    }

    std::unordered_map<id_type, std::size_t> file_words_map;
    const std::unordered_set<id_type>* p_files;

    std::unordered_set<string_type> lowered_word_set;
    lowered_word_set.reserve(word_set.size());

    for (auto& word : word_set) {
        string_type lowered_word(word);
        text_normalizer<char_type>::to_lower(lowered_word);
        lowered_word_set.emplace(std::move(lowered_word));
    }

    read_lock r_lock(rw_lock);

    for (auto& word : lowered_word_set) {
        std::pair<bool, id_type> word_result = get_file_set_for_word_unsafe(word, p_files);

        if (word_result.first == false) {
            return false; // If at least one word has no occurrences, the intersection is empty.
        }
        for (const auto file_id : *p_files) {
            ++file_words_map[file_id];
        }
    }

    std::size_t expected_count = lowered_word_set.size();
    for (const auto& [file_id, words_count] : file_words_map) {
        if (words_count == expected_count) {
            out_file_ids.emplace(file_id);

            out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
        }
    }

    return !out_file_ids.empty();
}

template<typename string_type>
inline bool index_manager<string_type>::get_file_set_for_lowered_word_set(const std::unordered_set<string_type>& word_set, std::unordered_set<id_type>& out_file_ids, std::unordered_map<id_type, const string_type&>& out_files_table) const {
    if (word_set.empty()) {
        return false;
    }

    std::unordered_map<id_type, std::size_t> file_words_map;
    const std::unordered_set<id_type>* p_files;

    read_lock r_lock(rw_lock);

    for (auto& word : word_set) {
        std::pair<bool, id_type> word_result = get_file_set_for_word_unsafe(word, p_files);

        if (word_result.first == false) {
            return false; // If at least one word has no occurrences, the intersection is empty.
        }
        for (const auto file_id : *p_files) {
            ++file_words_map[file_id];
        }
    }

    std::size_t expected_count = word_set.size();
    for (const auto& [file_id, words_count] : file_words_map) {
        if (words_count == expected_count) {
            out_file_ids.emplace(file_id);

            out_files_table.emplace(file_id, files_table.get_value_ñref_unsafe(file_id));
        }
    }

    return !out_file_ids.empty();
}

// read_file
template <typename string_type>
inline string_type index_manager<string_type>::read_file(const string_type& file_path) const {
    std::string content = read_file_as_utf8(file_path);

    if constexpr (std::is_same_v<string_type, std::string>) {
        return content;
    }
    else if constexpr (std::is_same_v<string_type, std::u8string>) {
        return string_type(reinterpret_cast<const char8_t*>(content.data()), content.size());
    }
    // This is actually much faster than opening the file in the 'wide' basic_ifstream
    // and reading it in the wide string_type from the start, but it still gives the correct wide string_type afterwards
    else {
        return utf_converter<char_type>::utf8_to_string_type(content);
    }
}

template<typename string_type>
inline std::string index_manager<string_type>::read_file_as_utf8(const string_type& file_path) const {
    std::ifstream file;

    if constexpr (std::is_same<char_type, char16_t>::value || std::is_same<char_type, char32_t>::value) {
        std::filesystem::path converted_file_path(file_path);
        file.open(converted_file_path.generic_wstring(), std::ios::binary | std::ios::ate);
    }
    else {
        file.open(file_path, std::ios::binary | std::ios::ate);
    }

    if (!file) {
        throw std::runtime_error("Failed to open the file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    content.resize_and_overwrite(size, [&](char* buf, std::size_t) {
        file.read(buf, size);
        return static_cast<std::size_t>(file.gcount());
    });

    return content;
}

// parse_and_normalize_words
template <typename string_type>
inline std::vector<string_type> index_manager<string_type>::parse_and_normalize_words(string_type&& content) const {
    std::vector<string_type> words;
    words.reserve(250); // Word count assumption

    string_type current_word;
    current_word.reserve(20);

    static auto& ctype = text_normalizer<char_type>::get_ctype();

    for (const char_type c : content) {
        if (ctype.is(std::ctype_base::alnum, c)) {
            current_word += ctype.tolower(c);
        }
        else if (!current_word.empty()) {
            words.emplace_back(std::move(current_word));
            current_word.clear();
        }
    }

    if (!current_word.empty()) {
        words.emplace_back(std::move(current_word));
    }

    return words;
}

// parse_and_normalize_words_ss
template <typename string_type>
inline std::vector<string_type> index_manager<string_type>::parse_and_normalize_words_ss(string_type&& content) const {
    text_normalizer<char_type>::to_lower(content);

    thread_local std::basic_stringstream<char_type> ss([] {
        std::basic_stringstream<char_type> local_ss;
        local_ss.imbue(text_normalizer<char_type>::new_ss_locale());
        return local_ss;
    }());

    ss.str(content);
    ss.clear();

    std::vector<string_type> words;
    words.reserve(250); // Word count assumption

    string_type token;
    while (ss >> token) {
        words.emplace_back(std::move(token));
    }

    return words;
}
