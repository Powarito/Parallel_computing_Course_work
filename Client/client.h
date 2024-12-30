#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WinBase.h>
#include <climits>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include "project_types.h"
#include "utility.h"
#include "word_entry.h"
#include "network_codes.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

template <typename string_type>
class client {
public:
    inline static constexpr bool is_host_big_endian();

    inline static void init_protocol();
    inline static void terminate_protocol();

    inline client(const std::string& ip_address, const int port);
    inline ~client();

    inline client(const client& other) = delete;
    inline client(client&& other) = delete;
    inline client& operator=(const client& other) = delete;
    inline client& operator=(client&& other) = delete;

public:
    // Send and recv data using the protocol. Return true if the connection was closed due to errors, otherwise close connection manually and return false (even if response != OK)
    inline bool do_set_new_writer_duration(float writer_duration, response& out_response);
    inline bool do_set_new_reader_duration(float reader_duration, response& out_response);
    inline bool do_get_writer_duration(float& out_writer_duration, response& out_response);
    inline bool do_get_reader_duration(float& out_reader_duration, response& out_response);
    inline bool do_get_file_content(const std::string& filename, std::string& out_file_content, response& out_response);
    inline bool do_get_write_result(big_id_type write_task_id, response& out_response);
    inline bool do_modify_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response);
    inline bool do_remove_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response);
    inline bool do_add_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response);
    inline bool do_add_create_file(const std::string& filename, const std::string& file_content, big_id_type& out_write_task_id, response& out_response);
    inline bool do_has_file(const std::string& filename, response& out_response);
    inline bool do_search(
        const std::unordered_set<string_type>& word_set,
        std::set<word_entry>& out_word_entries,
        std::map<id_type, std::string>& out_file_table,
        response& out_response
    );
    inline bool do_search_files_only(
        const std::unordered_set<string_type>& word_set,
        std::vector<std::string>& out_file_table,
        response& out_response
    );

private:
    inline void connect_to_server();
    inline static void close_connection(SOCKET client_socket);

    inline static void check_requirements();

    inline static bool recv_response_code(SOCKET client_socket, response& out_response);

    template <typename T>
    inline static int recv_integer_value(SOCKET client_socket, T& out_value);
    template <typename T>
    inline static int send_integer_value(SOCKET client_socket, T value);

    // Returns true if the connection was closed due to errors
    template <typename T>
    inline static bool recv_integer_value_and_handle(SOCKET client_socket, T& out_value);
    // Returns true if the connection was closed due to errors
    template <typename T>
    inline static bool send_integer_value_and_handle(SOCKET client_socket, T value);

    // Receives a UTF-8 string size and the string itself, convert it to string_type.
    // Returns true if the connection was closed due to errors
    inline static bool recv_size_and_string_and_handle(SOCKET client_socket, string_type& out_string);
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
    struct sockaddr_in server_addr;

    constexpr static bool is_big_endian = std::endian::native == std::endian::big;
};

template <typename string_type>
inline constexpr bool client<string_type>::is_host_big_endian() {
    return is_big_endian;
}

template <typename string_type>
inline void client<string_type>::init_protocol() {
    WSADATA wsa_data;
    if (int error_code = WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::string error_message = "WSAStartup (underlying API) failed: " + get_last_error_as_string(true, error_code) + ".";
        throw std::exception(error_message.c_str());
    }
}

template <typename string_type>
inline void client<string_type>::terminate_protocol() {
    WSACleanup();
}

template <typename string_type>
inline client<string_type>::client(const std::string& ip_address, const int port) {
    check_requirements();

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_addr.sin_port = htons(port);
}

template <typename string_type>
inline client<string_type>::~client() {
    //close_connection(m_socket);
}

template <typename string_type>
inline void client<string_type>::connect_to_server() {
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::string error_message = "Error creating socket: " + get_last_error_as_string() + ".";
        throw std::exception(error_message.c_str());
    }

    if (connect(m_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::string error_message = "CLIENT (CONNECT): Connect failed: " + get_last_error_as_string() + ".";
        throw std::exception(error_message.c_str());
    }
}

template <typename string_type>
inline void client<string_type>::close_connection(SOCKET client_socket) {
    closesocket(client_socket);
}

template <typename string_type>
inline void client<string_type>::check_requirements() {
    static_assert(sizeof(float) == 4 && CHAR_BIT == 8 && std::numeric_limits<float>::is_iec559);
    static_assert(sizeof(double) == 8 && CHAR_BIT == 8 && std::numeric_limits<double>::is_iec559);
    static_assert(sizeof(bool) == 1 && CHAR_BIT == 8);
}

template <typename string_type>
inline bool client<string_type>::do_set_new_writer_duration(float writer_duration, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::set_new_writer_duration);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    std::uint32_t writer_duration_integer = ieee754_to_integer(writer_duration);
    if (send_integer_value_and_handle(m_socket, writer_duration_integer)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_set_new_reader_duration(float reader_duration, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::set_new_reader_duration);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    std::uint32_t reader_duration_integer = ieee754_to_integer(reader_duration);
    if (send_integer_value_and_handle(m_socket, reader_duration_integer)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_get_writer_duration(float& out_writer_duration, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::get_writer_duration);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    std::uint32_t writer_duration_integer;
    if (recv_integer_value_and_handle(m_socket, writer_duration_integer)) {
        return true;
    }
    out_writer_duration = integer_to_ieee754(writer_duration_integer);

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_get_reader_duration(float& out_reader_duration, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::get_reader_duration);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    std::uint32_t reader_duration_integer;
    if (recv_integer_value_and_handle(m_socket, reader_duration_integer)) {
        return true;
    }
    out_reader_duration = integer_to_ieee754(reader_duration_integer);

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_get_file_content(const std::string& filename, std::string& out_file_content, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::get_file_content);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    if (recv_size_and_utf8_string_and_handle(m_socket, out_file_content)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_get_write_result(big_id_type write_task_id, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::get_write_result);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_integer_value_and_handle(m_socket, write_task_id)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_modify_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::modify_file);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    if (recv_integer_value_and_handle(m_socket, out_write_task_id)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_remove_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::remove_file);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    if (recv_integer_value_and_handle(m_socket, out_write_task_id)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_add_file(const std::string& filename, big_id_type& out_write_task_id, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::add_file);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    bool on_server_flag = true;
    if (send_integer_value_and_handle(m_socket, on_server_flag)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    if (recv_integer_value_and_handle(m_socket, out_write_task_id)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_add_create_file(const std::string& filename, const std::string& file_content, big_id_type& out_write_task_id, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::add_file);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    bool on_server_flag = false;
    if (send_integer_value_and_handle(m_socket, on_server_flag)) {
        return true;
    }

    if (send_size_and_utf8_string_and_handle(m_socket, file_content)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    if (recv_integer_value_and_handle(m_socket, out_write_task_id)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_has_file(const std::string& filename, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::has_file);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    if (send_size_and_utf8_string_and_handle(m_socket, filename)) {
        return true;
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::do_search(const std::unordered_set<string_type>& word_set, std::set<word_entry>& out_word_entries, std::map<id_type, std::string>& out_file_table, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::search);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    bool files_only = false;
    if (send_integer_value_and_handle(m_socket, files_only)) {
        return true;
    }

    std::uint16_t amount_of_words = word_set.size();
    if (send_integer_value_and_handle(m_socket, amount_of_words)) {
        return true;
    }

    for (const auto& word : word_set) {
        if (send_size_and_string_and_handle(m_socket, word)) {
            return true;
        }
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    id_type found_files_amount;
    if (recv_integer_value_and_handle(m_socket, found_files_amount)) {
        return true;
    }

    for (decltype(found_files_amount) file_idx = 0; file_idx < found_files_amount; ++file_idx) {
        id_type file_id;
        if (recv_integer_value_and_handle(m_socket, file_id)) {
            return true;
        }

        std::string filepath;
        if (recv_size_and_utf8_string_and_handle(m_socket, filepath)) {
            return true;
        }

        out_file_table.emplace(file_id, std::move(filepath));
    }

    std::uint64_t entries_amount;
    if (recv_integer_value_and_handle(m_socket, entries_amount)) {
        return true;
    }

    for (decltype(entries_amount) entry_idx = 0; entry_idx < entries_amount; ++entry_idx) {
        id_type file_id, position;
        if (recv_integer_value_and_handle(m_socket, file_id)) {
            return true;
        }
        if (recv_integer_value_and_handle(m_socket, position)) {
            return true;
        }
        out_word_entries.emplace(file_id, position);
    }

    close_connection(m_socket);
    return false;
}

template<typename string_type>
inline bool client<string_type>::do_search_files_only(const std::unordered_set<string_type>& word_set, std::vector<std::string>& out_file_table, response& out_response) {
    connect_to_server();

    // Send command
    code_type client_command = static_cast<code_type>(command::search);
    if (send_integer_value_and_handle(m_socket, client_command)) {
        return true;
    }

    // Send client data
    bool files_only = true;
    if (send_integer_value_and_handle(m_socket, files_only)) {
        return true;
    }

    std::uint16_t amount_of_words = word_set.size();
    if (send_integer_value_and_handle(m_socket, amount_of_words)) {
        return true;
    }

    for (const auto& word : word_set) {
        if (send_size_and_string_and_handle(m_socket, word)) {
            return true;
        }
    }

    // Receive results
    if (recv_response_code(m_socket, out_response)) {
        return true;
    }
    if (out_response != response::ok) {
        close_connection(m_socket);
        return false;
    }

    id_type found_files_amount;
    if (recv_integer_value_and_handle(m_socket, found_files_amount)) {
        return true;
    }

    out_file_table.reserve(found_files_amount);

    for (decltype(found_files_amount) file_idx = 0; file_idx < found_files_amount; ++file_idx) {
        std::string filepath;
        if (recv_size_and_utf8_string_and_handle(m_socket, filepath)) {
            return true;
        }

        out_file_table.emplace_back(std::move(filepath));
    }

    close_connection(m_socket);
    return false;
}

template <typename string_type>
inline bool client<string_type>::recv_response_code(SOCKET client_socket, response& out_response) {
    code_type to_recv_response_code;
    if (recv_integer_value_and_handle(client_socket, to_recv_response_code)) {
        return true;
    }

    out_response = static_cast<response>(to_recv_response_code);
    return false;
}

template <typename string_type>
template <typename T>
inline int client<string_type>::recv_integer_value(SOCKET client_socket, T& out_value) {
    char value_buffer[sizeof(out_value)];
    int recv_size = recv(client_socket, value_buffer, sizeof(value_buffer), 0);
    if (recv_size > 0) {
        out_value = from_big_endian<T>(value_buffer);
    }
    return recv_size;
}

template <typename string_type>
template <typename T>
inline int client<string_type>::send_integer_value(SOCKET client_socket, T value) {
    char value_buffer[sizeof(value)];
    to_big_endian<T>(value, value_buffer);

    return send(client_socket, value_buffer, sizeof(value_buffer), 0);
}

template <typename string_type>
template <typename T>
inline bool client<string_type>::recv_integer_value_and_handle(SOCKET client_socket, T& out_value) {
    int recv_size = recv_integer_value<T>(client_socket, out_value);
    if (recv_size <= 0) {
        close_connection(client_socket);
        return true;
    }
    return false;
}

template <typename string_type>
template <typename T>
inline bool client<string_type>::send_integer_value_and_handle(SOCKET client_socket, T value) {
    int send_size = send_integer_value<T>(client_socket, value);
    if (send_size <= 0) {
        close_connection(client_socket);
        return true;
    }
    return false;
}

template <typename string_type>
inline bool client<string_type>::recv_size_and_string_and_handle(SOCKET client_socket, string_type& out_string) {
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
            close_connection(client_socket);
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

    return false;
}

template <typename string_type>
inline bool client<string_type>::send_size_and_string_and_handle(SOCKET client_socket, const string_type& string) {
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
inline bool client<string_type>::recv_size_and_utf8_string_and_handle(SOCKET client_socket, std::string& out_string) {
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
inline bool client<string_type>::send_size_and_utf8_string_and_handle(SOCKET client_socket, const std::string& string) {
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
inline std::string client<string_type>::get_last_error_as_string(bool pass_error_code, int error_code) {
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
