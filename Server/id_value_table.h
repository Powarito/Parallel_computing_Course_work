#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <stdexcept>
#include "concurrent_utility.h"
#include "utility.h"

template <typename id_type, typename value_type, bool double_sided = true>
class id_value_table {
public:
    // Add a new value, assign it a unique ID and return it
    inline id_type add_value(const value_type& new_value);
    inline id_type add_value(value_type&& new_value);
    inline id_type add_value_unsafe(const value_type& new_value);
    inline id_type add_value_unsafe(value_type&& new_value);

    template <bool T = double_sided, typename = std::enable_if_t<T == false>>
    inline void modify_by_id(id_type value_id, const value_type& new_value);
    template <bool T = double_sided, typename = std::enable_if_t<T == false>>
    inline void modify_by_id(id_type value_id, value_type&& new_value);
    template <bool T = double_sided, typename = std::enable_if_t<T == false>>
    inline void modify_by_id_unsafe(id_type value_id, const value_type& new_value);
    template <bool T = double_sided, typename = std::enable_if_t<T == false>>
    inline void modify_by_id_unsafe(id_type value_id, value_type&& new_value);

    inline void remove_by_id(id_type value_id);
    inline void remove_by_id_unsafe(id_type value_id);

    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline void remove_by_value(const value_type& value_target);
    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline void remove_by_value_unsafe(const value_type& value_target);

    inline value_type get_value(id_type value_id) const;
    inline value_type get_value_unsafe(id_type value_id) const;

    inline const value_type& get_value_ñref(id_type value_id) const;
    inline const value_type& get_value_ñref_unsafe(id_type value_id) const;

    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline id_type get_value_id(const value_type& value_target) const;
    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline id_type get_value_id_unsafe(const value_type& value_target) const;

    // Returns 0 if not found instead of throwing an exception
    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline id_type get_value_id_always(const value_type& value_target) const;
    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline id_type get_value_id_always_unsafe(const value_type& value_target) const;

    inline bool has_id(id_type value_id) const;
    inline bool has_id_unsafe(id_type value_id) const;

    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline bool has_value(const value_type& value_target) const;
    template <bool T = double_sided, typename = std::enable_if_t<T>>
    inline bool has_value_unsafe(const value_type& value_target) const;

    inline void clear();
    inline void clear_unsafe();

private:
    template <typename U>
    inline id_type do_add_value_unsafe(U&& new_value);

    template <typename U, bool T = double_sided, typename = std::enable_if_t<T == false>>
    inline void do_modify_by_id_unsafe(id_type value_id, U&& new_value);

private:
    // ID -> value
    std::unordered_map<id_type, value_type> id_to_value;

    // value -> ID
    std::conditional_t<double_sided, std::unordered_map<value_type, id_type>, std::nullptr_t> value_to_id;

    id_type next_id = 1;

    mutable read_write_lock rw_lock;
};

// add_value
template <typename id_type, typename value_type, bool double_sided>
inline id_type id_value_table<id_type, value_type, double_sided>::add_value(const value_type& new_value) {
    write_lock w_lock(rw_lock);
    return add_value_unsafe(new_value);
}

template <typename id_type, typename value_type, bool double_sided>
inline id_type id_value_table<id_type, value_type, double_sided>::add_value(value_type&& new_value) {
    write_lock w_lock(rw_lock);
    return add_value_unsafe(std::move(new_value));
}

template <typename id_type, typename value_type, bool double_sided>
inline id_type id_value_table<id_type, value_type, double_sided>::add_value_unsafe(const value_type& new_value) {
    return do_add_value_unsafe(new_value);
}

template <typename id_type, typename value_type, bool double_sided>
inline id_type id_value_table<id_type, value_type, double_sided>::add_value_unsafe(value_type&& new_value) {
    return do_add_value_unsafe(std::move(new_value));
}

template <typename id_type, typename value_type, bool double_sided>
template <typename U>
inline id_type id_value_table<id_type, value_type, double_sided>::do_add_value_unsafe(U&& new_value) {
    if constexpr (double_sided) {
        if (value_to_id.find(new_value) != value_to_id.end()) {
            throw std::invalid_argument("Value already exists.");
        }
    }

    id_type value_id = next_id++;
    if constexpr (double_sided) {
        // This does NOT causes move semantics, only copying
        value_to_id[new_value] = value_id;
    }
    id_to_value[value_id] = std::forward<U>(new_value);

    return value_id;
}

// modify_by_id
template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::modify_by_id(id_type value_id, const value_type& new_value) {
    write_lock w_lock(rw_lock);
    modify_by_id_unsafe(value_id, new_value);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::modify_by_id(id_type value_id, value_type&& new_value) {
    write_lock w_lock(rw_lock);
    modify_by_id_unsafe(value_id, std::move(new_value));
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::modify_by_id_unsafe(id_type value_id, const value_type& new_value) {
    do_modify_by_id_unsafe(value_id, new_value);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::modify_by_id_unsafe(id_type value_id, value_type&& new_value) {
    do_modify_by_id_unsafe(value_id, std::move(new_value));
}

template <typename id_type, typename value_type, bool double_sided>
template <typename U, bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::do_modify_by_id_unsafe(id_type value_id, U&& new_value) {
    auto it = id_to_value.find(value_id);
    if (it == id_to_value.end()) {
        throw std::out_of_range("Value ID not found.");
    }

    it->second = std::forward<U>(new_value);
}

// remove_by_id
template <typename id_type, typename value_type, bool double_sided>
inline void id_value_table<id_type, value_type, double_sided>::remove_by_id(id_type value_id) {
    write_lock w_lock(rw_lock);
    remove_by_id_unsafe(value_id);
}

template <typename id_type, typename value_type, bool double_sided>
inline void id_value_table<id_type, value_type, double_sided>::remove_by_id_unsafe(id_type value_id) {
    auto it = id_to_value.find(value_id);
    if (it == id_to_value.end()) {
        throw std::out_of_range("Value ID not found.");
    }

    if constexpr (double_sided) {
        value_to_id.erase(it->second);
    }
    id_to_value.erase(it);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::remove_by_value(const value_type& value_target) {
    write_lock w_lock(rw_lock);
    remove_by_value_unsafe(value_target);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline void id_value_table<id_type, value_type, double_sided>::remove_by_value_unsafe(const value_type& value_target) {
    auto it = value_to_id.find(value_target);
    if (it == value_to_id.end()) {
        throw std::out_of_range("Value not found.");
    }

    id_to_value.erase(it->second);
    value_to_id.erase(it);
}

// get_value
template <typename id_type, typename value_type, bool double_sided>
inline value_type id_value_table<id_type, value_type, double_sided>::get_value(id_type value_id) const {
    return get_value_ñref(value_id);
}

template <typename id_type, typename value_type, bool double_sided>
inline value_type id_value_table<id_type, value_type, double_sided>::get_value_unsafe(id_type value_id) const {
    return get_value_ñref_unsafe(value_id);
}

template <typename id_type, typename value_type, bool double_sided>
inline const value_type& id_value_table<id_type, value_type, double_sided>::get_value_ñref(id_type value_id) const {
    read_lock r_lock(rw_lock);
    return get_value_ñref_unsafe(value_id);
}

template <typename id_type, typename value_type, bool double_sided>
inline const value_type& id_value_table<id_type, value_type, double_sided>::get_value_ñref_unsafe(id_type value_id) const {
    auto it = id_to_value.find(value_id);
    if (it == id_to_value.end()) {
        throw std::out_of_range("Value ID not found.");
    }
    return it->second;
}

// get_value_id
template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline id_type id_value_table<id_type, value_type, double_sided>::get_value_id(const value_type& value_target) const {
    read_lock r_lock(rw_lock);
    return get_value_id_unsafe(value_target);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline id_type id_value_table<id_type, value_type, double_sided>::get_value_id_unsafe(const value_type& value_target) const {
    auto it = value_to_id.find(value_target);
    if (it == value_to_id.end()) {
        throw std::out_of_range("Value not found.");
    }
    return it->second;
}

// get_value_id_always
template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline id_type id_value_table<id_type, value_type, double_sided>::get_value_id_always(const value_type& value_target) const {
    read_lock r_lock(rw_lock);
    return get_value_id_always_unsafe(value_target);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline id_type id_value_table<id_type, value_type, double_sided>::get_value_id_always_unsafe(const value_type& value_target) const {
    auto it = value_to_id.find(value_target);
    if (it == value_to_id.end()) {
        return 0;
    }
    return it->second;
}

// has_id
template <typename id_type, typename value_type, bool double_sided>
inline bool id_value_table<id_type, value_type, double_sided>::has_id(id_type value_id) const {
    read_lock r_lock(rw_lock);
    return has_id_unsafe(value_id);
}

template <typename id_type, typename value_type, bool double_sided>
inline bool id_value_table<id_type, value_type, double_sided>::has_id_unsafe(id_type value_id) const {
    return id_to_value.find(value_id) != id_to_value.end();
}

// has_value
template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline bool id_value_table<id_type, value_type, double_sided>::has_value(const value_type& value_target) const {
    read_lock r_lock(rw_lock);
    return has_value_unsafe(value_target);
}

template <typename id_type, typename value_type, bool double_sided>
template <bool T, typename>
inline bool id_value_table<id_type, value_type, double_sided>::has_value_unsafe(const value_type& value_target) const {
    return value_to_id.find(value_target) != value_to_id.end();
}

// clear
template <typename id_type, typename value_type, bool double_sided>
inline void id_value_table<id_type, value_type, double_sided>::clear() {
    write_lock w_lock(rw_lock);
    clear_unsafe();
}

template <typename id_type, typename value_type, bool double_sided>
inline void id_value_table<id_type, value_type, double_sided>::clear_unsafe() {
    id_to_value.clear();
    if constexpr (double_sided) {
        value_to_id.clear();
    }
    next_id = 1;
}
