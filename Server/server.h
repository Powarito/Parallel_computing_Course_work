#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WinBase.h>
#include <climits>
#include <filesystem>
#include "index_manager.h"
#include "rw_scheduled_thread_pool.h"
#include "network_codes.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

template <typename string_type>
class server {
public:
    inline static constexpr bool is_host_big_endian();

    inline static void init_protocol();
    inline static void terminate_protocol();

    inline server();
    inline ~server();

    inline server(const server& other) = delete;
    inline server(server&& other) = delete;
    inline server& operator=(const server& other) = delete;
    inline server& operator=(server&& other) = delete;

    inline void init_server(const std::string& ip_address, const int port) const;

    inline SOCKET get_socket() const;
    inline index_manager<string_type>& get_index();
    inline rw_scheduled_thread_pool& get_thread_pool();
    inline id_value_table<big_id_type, response, false>& get_write_tasks_statuses();
    inline std::filesystem::path get_base_dir() const;

    inline void on_client_accepted(SOCKET client_socket);
    inline void serve_client(SOCKET client_socket);

private:
    inline static void close_connection(SOCKET client_socket);

    inline static void check_requirements();

    inline void build_index(bool clear_present = false);

    inline static void do_index_set_new_writer_duration(SOCKET client_socket, server& this_server);
    inline static void do_index_set_new_reader_duration(SOCKET client_socket, server& this_server);
    inline static void do_index_get_writer_duration(SOCKET client_socket, server& this_server);
    inline static void do_index_get_reader_duration(SOCKET client_socket, server& this_server);
    inline static void do_index_get_file_content(SOCKET client_socket, server& this_server);
    inline static void do_index_get_write_result(SOCKET client_socket, server& this_server);
    inline static void do_index_modify_file(SOCKET client_socket, server& this_server);
    inline static void do_index_remove_file(SOCKET client_socket, server& this_server);
    inline static void do_index_add_file(SOCKET client_socket, server& this_server);
    inline static void do_index_has_file(SOCKET client_socket, server& this_server);
    inline static void do_index_search(SOCKET client_socket, server& this_server);

    inline static void do_index_add_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path);
    inline static void do_index_add_create_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path, string_type&& file_content);
    inline static void do_index_remove_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path);
    inline static void do_index_modify_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path);

    inline static void send_responce_code(SOCKET client_socket, response responce_code);
    inline static void send_responce_code_and_close(SOCKET client_socket, response responce_code);

    template <typename T>
    inline static int recv_integer_value(SOCKET client_socket, T& out_value);
    template <typename T>
    inline static int send_integer_value(SOCKET client_socket, T value);

    // Returns true if the connection was closed due to errors
    template <typename T>
    inline static bool recv_integer_value_and_handle(SOCKET client_socket, T& out_value, bool can_be_zero = false);
    // Returns true if the connection was closed due to errors
    template <typename T>
    inline static bool send_integer_value_and_handle(SOCKET client_socket, T value);

    // Receives a UTF-8 string size and the string itself, convert it to string_type and, if tolower == true, lower it.
    // Returns true if the connection was closed due to errors
    inline static bool recv_size_and_string_and_handle(SOCKET client_socket, string_type& out_string, bool tolower = true);
    // Converts the string of type string_type to a UTF-8 string and sends the string size and the string itself over the network.
    // Returns true if the connection was closed due to errors
    inline static bool send_size_and_string_and_handle(SOCKET client_socket, const string_type& string);

    // Receives a UTF-8 string size and the string itself.
    // Returns true if the connection was closed due to errors
    inline static bool recv_size_and_utf8_string_and_handle(SOCKET client_socket, std::string& out_string);
    // Sends the string size and the UTF-8 string itself over the network.
    // Returns true if the connection was closed due to errors
    inline static bool send_size_and_utf8_string_and_handle(SOCKET client_socket, const std::string& string);

    inline static std::string get_last_error_as_string(bool pass_error_code = false, int error_code = 0);

private:
    SOCKET m_socket;

    constexpr static bool is_big_endian = std::endian::native == std::endian::big;

    index_manager<string_type> index;
    std::filesystem::path base_dir = "text_files";

    rw_scheduled_thread_pool thread_pool;
    id_value_table<big_id_type, response, false> write_tasks_statuses;

    static inline const std::unordered_map<code_type, std::function<void(SOCKET, server<string_type>&)>> function_map = {
        { static_cast<code_type>(command::set_new_writer_duration), &server<string_type>::do_index_set_new_writer_duration},
        { static_cast<code_type>(command::set_new_reader_duration), &server<string_type>::do_index_set_new_reader_duration},
        { static_cast<code_type>(command::get_writer_duration), &server<string_type>::do_index_get_writer_duration},
        { static_cast<code_type>(command::get_reader_duration), &server<string_type>::do_index_get_reader_duration},
        { static_cast<code_type>(command::get_file_content), &server<string_type>::do_index_get_file_content},
        { static_cast<code_type>(command::get_write_result), &server<string_type>::do_index_get_write_result},
        { static_cast<code_type>(command::modify_file), &server<string_type>::do_index_modify_file},
        { static_cast<code_type>(command::remove_file), &server<string_type>::do_index_remove_file},
        { static_cast<code_type>(command::add_file), &server<string_type>::do_index_add_file},
        { static_cast<code_type>(command::has_file), &server<string_type>::do_index_has_file},
        { static_cast<code_type>(command::search), &server<string_type>::do_index_search},
    };
};

template <typename string_type>
inline constexpr bool server<string_type>::is_host_big_endian() {
    return is_big_endian;
}

template <typename string_type>
inline void server<string_type>::init_protocol() {
    WSADATA wsa_data;
    if (int error_code = WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::string error_message = "WSAStartup (underlying API) failed: " + get_last_error_as_string(true, error_code) + ".";
        throw std::exception(error_message.c_str());
    }
}

template <typename string_type>
inline void server<string_type>::terminate_protocol() {
    WSACleanup();
}

template <typename string_type>
inline server<string_type>::server() {
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::string error_message = "Error creating socket: " + get_last_error_as_string() + ".";
        throw std::exception(error_message.c_str());
    }

    check_requirements();

    auto start1 = std::chrono::high_resolution_clock::now();
    build_index();
    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1);
    std::cout << "(Building index)\nTime : " << time1.count() << " ns  | " << time1.count() / 1'000'000 << " ms\n";

    thread_pool.initialize(std::thread::hardware_concurrency(), 0.5f, 7.5f);
}

template <typename string_type>
inline server<string_type>::~server() {
    close_connection(m_socket);
}

template <typename string_type>
inline void server<string_type>::init_server(const std::string& ip_address, const int port) const {
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_address.sin_port = htons(port);

    if (bind(m_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::string error_message = "SERVER (BIND): " + ip_address + ", port: " + std::to_string(port) + " - Bind failed: " + get_last_error_as_string() + ".";
        throw std::exception(error_message.c_str());
    }

    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::string error_message = "SERVER (LISTEN): Listen failed: " + get_last_error_as_string() + ".";
        throw std::exception(error_message.c_str());
    }
}

template <typename string_type>
inline SOCKET server<string_type>::get_socket() const {
    return m_socket;
}

template <typename string_type>
inline index_manager<string_type>& server<string_type>::get_index() {
    return index;
}

template <typename string_type>
inline rw_scheduled_thread_pool& server<string_type>::get_thread_pool() {
    return thread_pool;
}

template <typename string_type>
inline id_value_table<big_id_type, response, false>& server<string_type>::get_write_tasks_statuses() {
    return write_tasks_statuses;
}

template <typename string_type>
inline std::filesystem::path server<string_type>::get_base_dir() const {
    return base_dir;
}

template<typename string_type>
inline void server<string_type>::on_client_accepted(SOCKET client_socket) {
    thread_pool.add_reader_task(&server<string_type>::serve_client, this, client_socket);
}

template <typename string_type>
inline void server<string_type>::serve_client(SOCKET client_socket) {
    //send_responce_code(client_socket, response::ok); // For stress testing

    code_type to_recv_command_code = 0;
    int recv_size = recv_integer_value(client_socket, to_recv_command_code);
    if (recv_size < sizeof(to_recv_command_code)) {
        send_responce_code_and_close(client_socket, response::error_receiving_command);
        return;
    }

    // Process client command
    if (auto it = function_map.find(to_recv_command_code); it != function_map.end()) {
        it->second(client_socket, *this);
        // Don't call close_connection here. Do it in the function
    }
    else {
        send_responce_code_and_close(client_socket, response::invalid_command);
    }
}

template <typename string_type>
inline void server<string_type>::close_connection(SOCKET client_socket) {
    //std::cout << "connection closed\n";
    closesocket(client_socket);
}

template <typename string_type>
inline void server<string_type>::check_requirements() {
    static_assert(sizeof(float) == 4 && CHAR_BIT == 8 && std::numeric_limits<float>::is_iec559);
    static_assert(sizeof(double) == 8 && CHAR_BIT == 8 && std::numeric_limits<double>::is_iec559);
    static_assert(sizeof(bool) == 1 && CHAR_BIT == 8);
}

template <typename string_type>
inline void server<string_type>::build_index(bool clear_present) {
    if (clear_present) {
        index.clear_all();
    }

    std::filesystem::path canonical_base = std::filesystem::canonical(base_dir);
    using char_type = string_type::value_type;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(canonical_base)) {
        if (entry.is_regular_file()) {
            std::filesystem::path relative_path = std::filesystem::relative(entry.path(), canonical_base);
            std::filesystem::path full_path = base_dir / relative_path;

            if constexpr (std::is_same<char_type, char>::value) {
                index.add_file(full_path.generic_string());
            }
            if constexpr (std::is_same<char_type, char8_t>::value) {
                index.add_file(full_path.generic_u8string());
            }
            else if constexpr (std::is_same<char_type, wchar_t>::value) {
                index.add_file(full_path.generic_wstring());
            }
            else if constexpr (std::is_same<char_type, char16_t>::value) {
                index.add_file(full_path.generic_u16string());
            }
            else if constexpr (std::is_same<char_type, char32_t>::value) {
                index.add_file(full_path.generic_u32string());
            }
        }
    }
}

template <typename string_type>
inline void server<string_type>::do_index_set_new_writer_duration(SOCKET client_socket, server& this_server) {
    // Receive client data
    std::uint32_t writer_duration_integer;
    if (recv_integer_value_and_handle(client_socket, writer_duration_integer)) {
        return;
    }
    float writer_duration = integer_to_ieee754(writer_duration_integer);

    // Do query
    bool success = this_server.get_thread_pool().set_writer_duration(writer_duration);

    // Send results
    if (success) {
        send_responce_code_and_close(client_socket, response::ok);
    }
    else {
        send_responce_code_and_close(client_socket, response::new_duration_is_way_too_small);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_set_new_reader_duration(SOCKET client_socket, server& this_server) {
    // Receive client data
    std::uint32_t reader_duration_integer;
    if (recv_integer_value_and_handle(client_socket, reader_duration_integer)) {
        return;
    }
    float reader_duration = integer_to_ieee754(reader_duration_integer);

    // Do query
    bool success = this_server.get_thread_pool().set_reader_duration(reader_duration);

    // Send results
    if (success) {
        send_responce_code_and_close(client_socket, response::ok);
    }
    else {
        send_responce_code_and_close(client_socket, response::new_duration_is_way_too_small);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_get_writer_duration(SOCKET client_socket, server& this_server) {
    // Do query
    float writer_duration = this_server.get_thread_pool().get_writer_duration();
    std::uint32_t writer_duration_integer = ieee754_to_integer(writer_duration);

    // Send results
    send_responce_code(client_socket, response::ok);
    if (send_integer_value_and_handle(client_socket, writer_duration_integer) == false) {
        close_connection(client_socket);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_get_reader_duration(SOCKET client_socket, server& this_server) {
    // Do query
    float reader_duration = this_server.get_thread_pool().get_reader_duration();
    std::uint32_t reader_duration_integer = ieee754_to_integer(reader_duration);
    
    // Send results
    send_responce_code(client_socket, response::ok);
    if (send_integer_value_and_handle(client_socket, reader_duration_integer) == false) {
        close_connection(client_socket);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_get_file_content(SOCKET client_socket, server& this_server) {
    // Receive client data
    string_type filename;
    if (recv_size_and_string_and_handle(client_socket, filename, false)) {
        return;
    }

    // Do query
    std::string file_content;
    bool file_exist = this_server.get_index().get_file_content_utf8(filename, file_content);

    // Send results
    if (file_exist) {
        send_responce_code(client_socket, response::ok);

        if (send_size_and_utf8_string_and_handle(client_socket, file_content)) {
            return;
        }

        close_connection(client_socket);
    }
    else {
        send_responce_code_and_close(client_socket, response::file_not_found);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_get_write_result(SOCKET client_socket, server& this_server) {
    // Receive client data
    big_id_type write_task_id;
    if (recv_integer_value_and_handle(client_socket, write_task_id)) {
        return;
    }

    // Do query
    response result;
    try {
        result = this_server.get_write_tasks_statuses().get_value(write_task_id);
    }
    catch (std::exception&) {
        result = response::write_task_id_not_found;
    }

    // Send results
    send_responce_code_and_close(client_socket, result);
}

template <typename string_type>
inline void server<string_type>::do_index_modify_file(SOCKET client_socket, server& this_server) {
    // Receive client data
    string_type filename;
    if (recv_size_and_string_and_handle(client_socket, filename, false)) {
        return;
    }

    // Add write operation to the write scheduled queue
    big_id_type write_task_id = this_server.get_write_tasks_statuses().add_value(response::operation_is_not_processed);

    this_server.get_thread_pool().add_writer_task([&this_server, write_task_id, filename_obj = std::move(filename)]() mutable {
        do_index_modify_file_in_write_queue(this_server, write_task_id, std::move(filename_obj));
        }
    );

    // Send info about a successfully added write operation to the queue
    send_responce_code(client_socket, response::ok);
    if (send_integer_value_and_handle(client_socket, write_task_id) == false) {
        close_connection(client_socket);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_remove_file(SOCKET client_socket, server& this_server) {
    // Receive client data
    string_type filename;
    if (recv_size_and_string_and_handle(client_socket, filename, false)) {
        return;
    }

    // Add write operation to the write scheduled queue
    big_id_type write_task_id = this_server.get_write_tasks_statuses().add_value(response::operation_is_not_processed);

    this_server.get_thread_pool().add_writer_task([&this_server, write_task_id, filename_obj = std::move(filename)]() mutable {
        do_index_remove_file_in_write_queue(this_server, write_task_id, std::move(filename_obj));
        }
    );

    // Send info about a successfully added write operation to the queue
    send_responce_code(client_socket, response::ok);
    if (send_integer_value_and_handle(client_socket, write_task_id) == false) {
        close_connection(client_socket);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_add_file(SOCKET client_socket, server& this_server) {
    // Receive client data
    string_type filename;
    if (recv_size_and_string_and_handle(client_socket, filename, false)) {
        return;
    }

    bool on_server_flag = true;
    if (recv_integer_value_and_handle(client_socket, on_server_flag, true)) {
        return;
    }

    string_type file_content;
    if (on_server_flag == false && recv_size_and_string_and_handle(client_socket, file_content, false)) {
        return;
    }

    // Add write operation to the write scheduled queue
    big_id_type write_task_id = this_server.get_write_tasks_statuses().add_value(response::operation_is_not_processed);

    if (on_server_flag == false) {
        using char_type = string_type::value_type;

        // Add the server base directory as a prefix to the file path
        std::filesystem::path file_path = this_server.get_base_dir() / std::filesystem::path(filename);
        if constexpr (std::is_same<char_type, char>::value) {
            filename = file_path.generic_string();
        }
        if constexpr (std::is_same<char_type, char8_t>::value) {
            filename = file_path.generic_u8string();
        }
        else if constexpr (std::is_same<char_type, wchar_t>::value) {
            filename = file_path.generic_wstring();
        }
        else if constexpr (std::is_same<char_type, char16_t>::value) {
            filename = file_path.generic_u16string();
        }
        else if constexpr (std::is_same<char_type, char32_t>::value) {
            filename = file_path.generic_u32string();
        }

        this_server.get_thread_pool().add_writer_task([&this_server, write_task_id, filename_obj = std::move(filename), file_content_obj = std::move(file_content)]() mutable {
            do_index_add_create_file_in_write_queue(this_server, write_task_id, std::move(filename_obj), std::move(file_content_obj));
            }
        );
    }
    else {
        this_server.get_thread_pool().add_writer_task( [&this_server, write_task_id, filename_obj = std::move(filename)]() mutable {
            do_index_add_file_in_write_queue(this_server, write_task_id, std::move(filename_obj));
            }
        );
    }

    // Send info about a successfully added write operation to the queue
    send_responce_code(client_socket, response::ok);
    if (send_integer_value_and_handle(client_socket, write_task_id) == false) {
        close_connection(client_socket);
    }
}

template <typename string_type>
inline void server<string_type>::do_index_has_file(SOCKET client_socket, server& this_server) {
    // Receive client data
    string_type filename;
    if (recv_size_and_string_and_handle(client_socket, filename)) {
        return;
    }

    // Do query
    std::pair<bool, id_type> found_with_id = this_server.get_index().has_file(std::move(filename));
    bool found = found_with_id.first;

    // Send results
    if (found) {
        send_responce_code_and_close(client_socket, response::ok);
    }
    else {
        send_responce_code_and_close(client_socket, response::file_not_found);
    }

    return;
}

template <typename string_type>
inline void server<string_type>::do_index_search(SOCKET client_socket, server& this_server) {
    // Receive client data
    bool files_only = true;
    if (recv_integer_value_and_handle(client_socket, files_only, true)) {
        return;
    }

    std::uint16_t amount_of_words;
    if (recv_integer_value_and_handle(client_socket, amount_of_words)) {
        return;
    }

    using char_type = string_type::value_type;

    string_type lowered_word;
    std::unordered_set<string_type> lowered_word_set;
    lowered_word_set.reserve(amount_of_words);

    for (decltype(amount_of_words) word_idx = 0; word_idx < amount_of_words; ++word_idx) {
        if (recv_size_and_string_and_handle(client_socket, lowered_word)) { // lowered_word is assigned with a new string
            return;
        }

        lowered_word_set.emplace(std::move(lowered_word));
    }

    //send_responce_code(client_socket, response::ok); // For stress test
    // Do query
    std::unordered_set<word_entry> out_word_entries;
    std::unordered_set<id_type> out_file_ids;
    const std::unordered_set<word_entry>* cp_out_word_entries = nullptr;
    const std::unordered_set<id_type>* cp_out_file_ids = nullptr;

    std::unordered_map<id_type, const string_type&> out_files_table;

    bool found = false;
    std::pair<bool, id_type> found_with_id;

    bool more_than_one_word = lowered_word_set.size() > 1;
    if (more_than_one_word) {
        if (files_only) {
            found = this_server.get_index().get_file_set_for_lowered_word_set(lowered_word_set, out_file_ids, out_files_table);
        }
        else {
            found = this_server.get_index().get_word_entry_set_for_lowered_word_set(lowered_word_set, out_word_entries, out_files_table);
        }
    }
    else {
        auto single_element = std::begin(lowered_word_set);

        if (files_only) {
            found_with_id = this_server.get_index().get_file_set_for_lowered_word(*single_element, cp_out_file_ids, out_files_table);
            found = found_with_id.first;
        }
        else {
            found_with_id = this_server.get_index().get_word_entry_set_for_lowered_word(*single_element, cp_out_word_entries, out_files_table);
            found = found_with_id.first;
        }
    }

    id_type found_files_amount = out_files_table.size();

    // Send results
    if (found) {
        send_responce_code(client_socket, response::ok);
        if (send_integer_value_and_handle(client_socket, found_files_amount)) {
            return;
        }

        if (files_only) {
            for (const auto& [file_id, filepath] : out_files_table) {
                if (send_size_and_string_and_handle(client_socket, filepath)) {
                    return;
                }
            }
        }
        else {
            for (const auto& [file_id, filepath] : out_files_table) {
                if (send_integer_value_and_handle(client_socket, file_id)) {
                    return;
                }
                if (send_size_and_string_and_handle(client_socket, filepath)) {
                    return;
                }
            }

            const std::unordered_set<word_entry>& word_entries = more_than_one_word ? out_word_entries : *cp_out_word_entries;

            std::uint64_t entries_amount = word_entries.size();
            if (send_integer_value_and_handle(client_socket, entries_amount)) {
                return;
            }

            for (const auto& entry : word_entries) {
                send_integer_value(client_socket, entry.file_id);
                send_integer_value(client_socket, entry.position);
            }
        }

        close_connection(client_socket);
    }
    else {
        send_responce_code_and_close(client_socket, response::search_query_entries_not_found);
    }

    return;
}

template <typename string_type>
inline void server<string_type>::do_index_add_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path) {
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, response::operation_is_in_progress);

    bool added_file = this_server.get_index().add_file(std::move(file_path));

    response done_response = added_file ? response::ok : response::could_not_add_file;
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, done_response);
}

template <typename string_type>
inline void server<string_type>::do_index_add_create_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path, string_type&& file_content) {
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, response::operation_is_in_progress);

    bool added_file = this_server.get_index().add_create_file(std::move(file_path), std::move(file_content));

    response done_response = added_file ? response::ok : response::could_not_add_file;
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, done_response);
}

template<typename string_type>
inline void server<string_type>::do_index_remove_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path) {
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, response::operation_is_in_progress);

    bool deleted_file = this_server.get_index().remove_file(std::move(file_path));

    response done_response = deleted_file ? response::ok : response::file_not_found;
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, done_response);
}

template<typename string_type>
inline void server<string_type>::do_index_modify_file_in_write_queue(server& this_server, big_id_type write_task_id, string_type&& file_path) {
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, response::operation_is_in_progress);

    bool modified_file = this_server.get_index().modify_file(std::move(file_path));

    response done_response = modified_file ? response::ok : response::file_not_found;
    this_server.get_write_tasks_statuses().modify_by_id(write_task_id, done_response);
}

template <typename string_type>
inline void server<string_type>::send_responce_code(SOCKET client_socket, response response_code) {
    code_type to_send_response_code = static_cast<code_type>(response_code);
    send_integer_value(client_socket, to_send_response_code);
}

template <typename string_type>
inline void server<string_type>::send_responce_code_and_close(SOCKET client_socket, response responce_code) {
    send_responce_code(client_socket, responce_code);
    close_connection(client_socket);
}

template <typename string_type>
template <typename T>
inline int server<string_type>::recv_integer_value(SOCKET client_socket, T& out_value) {
    char value_buffer[sizeof(out_value)];
    int recv_size = recv(client_socket, value_buffer, sizeof(value_buffer), 0);
    if (recv_size > 0) {
        out_value = from_big_endian<T>(value_buffer);
    }
    return recv_size;
}

template <typename string_type>
template <typename T>
inline int server<string_type>::send_integer_value(SOCKET client_socket, T value) {
    char value_buffer[sizeof(value)];
    to_big_endian<T>(value, value_buffer);

    return send(client_socket, value_buffer, sizeof(value_buffer), 0);
}

template <typename string_type>
template <typename T>
inline bool server<string_type>::recv_integer_value_and_handle(SOCKET client_socket, T& out_value, bool can_be_zero) {
    int recv_size = recv_integer_value<T>(client_socket, out_value);
    if (recv_size <= 0) {
        send_responce_code_and_close(client_socket, response::error_receiving_data);
        return true;
    }
    else if (can_be_zero == false && out_value == 0) {
        send_responce_code_and_close(client_socket, response::argument_is_zero);
        return true;
    }
    return false;
}

template <typename string_type>
template <typename T>
inline bool server<string_type>::send_integer_value_and_handle(SOCKET client_socket, T value) {
    int send_size = send_integer_value<T>(client_socket, value);
    if (send_size <= 0) {
        close_connection(client_socket);
        return true;
    }
    return false;
}

template <typename string_type>
inline bool server<string_type>::recv_size_and_string_and_handle(SOCKET client_socket, string_type& out_string, bool tolower) {
    std::uint16_t byte_size_string;
    if (recv_integer_value_and_handle(client_socket, byte_size_string)) {
        return true;
    }

    std::string utf8_string(byte_size_string, '\0');

    std::uint16_t total_received = 0;
    while (total_received < byte_size_string) {
        int bytes_to_receive = std::min(1024, byte_size_string - total_received);

        int recv_size = recv(client_socket, &utf8_string[total_received], bytes_to_receive, 0);
        if (recv_size <= 0) {
            send_responce_code_and_close(client_socket, response::error_receiving_data);
            return true;
        }
        total_received += recv_size;
    }

    using char_type = string_type::value_type;

    if constexpr (std::is_same<char_type, char>::value) {
        out_string = std::move(utf8_string);
    }
    else if constexpr (std::is_same<char_type, char8_t>::value) {
        out_string = string_type(reinterpret_cast<const char8_t*>(utf8_string.data()), utf8_string.size());
    }
    else {
        out_string = utf_converter<char_type>::utf8_to_string_type(utf8_string);
    }

    if (tolower) {
        text_normalizer<char_type>::to_lower(out_string);
    }

    return false;
}

template <typename string_type>
inline bool server<string_type>::send_size_and_string_and_handle(SOCKET client_socket, const string_type& string) {
    using char_type = string_type::value_type;

    std::string utf8_string;

    if constexpr (std::is_same<char_type, char>::value) {
        utf8_string = string;
    }
    else if constexpr (std::is_same<char_type, char8_t>::value) {
        utf8_string = string_type(reinterpret_cast<const char8_t*>(string.data()), string.size());
    }
    else {
        utf8_string = utf_converter<char_type>::string_type_to_utf8(string);
    }

    std::uint16_t byte_size_string = utf8_string.size();
    if (send_integer_value_and_handle(client_socket, byte_size_string)) {
        return true;
    }

    std::uint16_t total_sent = 0;
    while (total_sent < byte_size_string) {
        int bytes_to_send = std::min(1024, byte_size_string - total_sent);

        int send_size = send(client_socket, &utf8_string[total_sent], bytes_to_send, 0);
        if (send_size <= 0) {
            close_connection(client_socket);
            return true;
        }
        total_sent += send_size;
    }
    return false;
}

template<typename string_type>
inline bool server<string_type>::recv_size_and_utf8_string_and_handle(SOCKET client_socket, std::string& out_string) {
    std::uint16_t byte_size_string;
    if (recv_integer_value_and_handle(client_socket, byte_size_string)) {
        return true;
    }

    out_string.resize(byte_size_string);

    std::uint16_t total_received = 0;
    while (total_received < byte_size_string) {
        int bytes_to_receive = std::min(1024, byte_size_string - total_received);

        int recv_size = recv(client_socket, &out_string[total_received], bytes_to_receive, 0);
        if (recv_size <= 0) {
            close_connection(client_socket);
            return true;
        }
        total_received += recv_size;
    }

    return false;
}

template<typename string_type>
inline bool server<string_type>::send_size_and_utf8_string_and_handle(SOCKET client_socket, const std::string& string) {
    std::uint16_t byte_size_string = string.size();
    if (send_integer_value_and_handle(client_socket, byte_size_string)) {
        return true;
    }

    std::uint16_t total_sent = 0;
    while (total_sent < byte_size_string) {
        int bytes_to_send = std::min(1024, byte_size_string - total_sent);

        int send_size = send(client_socket, &string[total_sent], bytes_to_send, 0);
        if (send_size <= 0) {
            close_connection(client_socket);
            return true;
        }
        total_sent += send_size;
    }
    return false;
}

template <typename string_type>
inline std::string server<string_type>::get_last_error_as_string(bool pass_error_code, int error_code) {
    DWORD error_message_id = error_code;

    if (!pass_error_code) {
        error_message_id = ::GetLastError();
        if (error_message_id == 0) {
            return std::string(); // No error message has been recorded
        }
    }

    LPSTR message_buffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    std::size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_message_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

    // Copy the error message into a std::string.
    std::string message(message_buffer, size);

    // Free the Win32's string's buffer.
    LocalFree(message_buffer);

    return message;
}
