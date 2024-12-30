#pragma once

#include <iostream>
#include <functional>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include "client.h"

template <typename T = int>
inline T get_choice(bool print = true) {
    if (print) {
        std::cout << "Choose an option: ";
    }

    T choice;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

class menu_node {
public:
    inline menu_node(const std::string& title)
        : title(title) {};

    virtual inline ~menu_node() = default;

    virtual inline void execute() = 0;

    inline const std::string& get_title() {
        return title;
    }

protected:
    std::string title;
};

class base_menu : public menu_node {
public:
    inline base_menu(const std::string& title) 
        : menu_node(title) {}

    inline void add_option(std::unique_ptr<menu_node> action) {
        options.emplace_back(action->get_title(), std::move(action));
    }

    inline void execute() override {
        while (true) {
            display();

            int choice = get_choice();
            if (choice == 0) {
                break;
            }
            else if (choice > 0 && choice <= options.size()) {
                options[choice - 1].second->execute();
            }
            else {
                std::cout << "Invalid choice. Try again.\n";
            }
        }
    }

private:
    std::vector<std::pair<std::string, std::unique_ptr<menu_node>>> options;

    inline void display() const {
        if (title.empty() == false) {
            std::cout << "\n=== " << title << " ===\n";
        }
        for (std::size_t i = 0; i < options.size(); ++i) {
            std::cout << i + 1 << ". " << options[i].first << "\n";
        }
        std::cout << "0. Back\n";
    }
};

class action : public menu_node {
public:
    template <typename F>
    inline action(const std::string& title, F&& func)
        : menu_node(title), func(std::forward<F>(func)) {}

    inline void execute() override {
        if (title.empty() == false) {
            std::cout << "\n=== " << title << " ===\n";
        }
        func();
    }

private:
    std::function<void()> func;
};

template <typename string_type>
class program_menu {
public:
    inline program_menu(client<string_type>& local_client);
    inline ~program_menu() = default;

    inline program_menu(const program_menu& other) = delete;
    inline program_menu(program_menu&& other) = delete;
    inline program_menu& operator=(const program_menu& other) = delete;
    inline program_menu& operator=(program_menu&& other) = delete;

public:
    inline void open_menu();

private:
    inline void menu_search();
    inline void menu_has_file();
    inline void menu_add_file();
    inline void menu_remove_file();
    inline void menu_modify_file();
    inline void menu_get_write_result();
    inline void menu_get_file_content();
    inline void menu_get_reader_duration();
    inline void menu_get_writer_duration();
    inline void menu_set_new_reader_duration();
    inline void menu_set_new_writer_duration();

    inline void print_response_code(response response_code) const;

private:
    client<string_type>& local_client;

    inline static const std::unordered_map<response, std::string> response_map = {
        { response::ok, "OK" },
        { response::invalid_command, "invalid command" },
        { response::error_receiving_command, "error receiving command" },
        { response::error_receiving_data, "error receiving data" },
        { response::argument_is_zero, "error: argument can't be zero" },
        { response::search_query_entries_not_found, "entries not found" },
        { response::file_not_found, "file not found" },
        { response::could_not_add_file, "could not add file" },
        { response::new_duration_is_way_too_small, "error: new duration is way too small" },
        { response::operation_is_not_processed, "operation is not processed" },
        { response::operation_is_in_progress, "operation is in progress" },
        { response::write_task_id_not_found, "error: write task ID not found" },
    };
};

template<typename string_type>
inline program_menu<string_type>::program_menu(client<string_type>& local_client) 
    : local_client(local_client)
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::ios::sync_with_stdio(false); // Additionally to speed up streams (we'll be using only C++ streams)
}

template<typename string_type>
inline void program_menu<string_type>::open_menu() {
    auto main_menu = std::make_unique<base_menu>("Main Menu");

    main_menu->add_option(std::make_unique<action>("Search", [this]() { menu_search(); }));
    main_menu->add_option(std::make_unique<action>("Has File", [this]() { menu_has_file(); }));
    main_menu->add_option(std::make_unique<action>("Add File", [this]() { menu_add_file(); }));
    main_menu->add_option(std::make_unique<action>("Remove File", [this]() { menu_remove_file(); }));
    main_menu->add_option(std::make_unique<action>("Modify File", [this]() { menu_modify_file(); }));
    main_menu->add_option(std::make_unique<action>("Get Write Result", [this]() { menu_get_write_result(); }));
    main_menu->add_option(std::make_unique<action>("Get File Content", [this]() { menu_get_file_content(); }));
    main_menu->add_option(std::make_unique<action>("Get Reader Duration", [this]() { menu_get_reader_duration(); }));
    main_menu->add_option(std::make_unique<action>("Get Writer Duration", [this]() { menu_get_writer_duration(); }));
    main_menu->add_option(std::make_unique<action>("Set New Reader Duration", [this]() { menu_set_new_reader_duration(); }));
    main_menu->add_option(std::make_unique<action>("Set New Writer Duration", [this]() { menu_set_new_writer_duration(); }));

    main_menu->execute();
}

template<typename string_type>
inline void program_menu<string_type>::menu_search() {
    using char_type = string_type::value_type;

    std::cout << "Enter search query:\n";

    std::string utf8_search_query;
    std::getline(std::cin, utf8_search_query);

    string_type search_query = utf_converter<char_type>::utf8_to_string_type(utf8_search_query);

    std::unordered_set<string_type> word_set;

    string_type current_word;
    current_word.reserve(20);

    static auto& ctype = text_normalizer<char_type>::get_ctype();

    for (const char_type c : search_query) {
        if (ctype.is(std::ctype_base::alnum, c)) {
            current_word += ctype.tolower(c);
        }
        else if (!current_word.empty()) {
            word_set.emplace(std::move(current_word));
            current_word.clear();
        }
    }

    if (!current_word.empty()) {
        word_set.emplace(std::move(current_word));
    }

    std::cout << "\nAlso get words entries in files (y - yes / n - just files)?: ";

    char input;
    std::cin >> input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    input = std::tolower(input);

    std::set<word_entry> out_word_entries;
    std::map<id_type, std::string> out_file_table;
    std::vector<std::string> out_file_table_vec;
    response out_response;

    bool connection_error_occured = false;

    bool with_entries = input == 'y';
    if (with_entries) {
        connection_error_occured = local_client.do_search(word_set, out_word_entries, out_file_table, out_response);
    }
    else {
        connection_error_occured = local_client.do_search_files_only(word_set, out_file_table_vec, out_response);
    }

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }

    if (with_entries) {
        std::cout << "\n---------------"
                     "\nFiles with IDs:"
                     "\n---------------\n";
        for (const auto& [file_id, filename] : out_file_table) {
            std::cout << file_id << ". " << filename << "\n";
        }

        std::cout << "\n---------------------------------"
                     "\nEntries (file ID, word position):"
                     "\n---------------------------------\n";

        for (const auto& entry : out_word_entries) {
            std::cout << "(" << entry.file_id << ", " << entry.position << ")   ";
        }
        std::cout << "\n";
    }
    else {
        std::cout << "\n------"
                     "\nFiles:"
                     "\n------\n";
        for (id_type file_idx = 0; file_idx < out_file_table_vec.size(); ++file_idx) {
            std::cout << file_idx + 1 << ". " << out_file_table_vec[file_idx] << "\n";
        }
    }

    while (true) {
        std::cout << "\n1. Get File Content By ID\n";
        std::cout <<   "0. Back\n";

        int choice = get_choice<int>();
        if (choice == 0) {
            break;
        }
        else if (choice > 0 && choice <= 1) {
            std::cout << "Enter file ID from the table above: ";
            id_type file_id = get_choice<id_type>(false);

            std::string filename;
            bool valid_id = false;

            if (with_entries && out_file_table.find(file_id) != out_file_table.end()) {
                valid_id = true;
                filename = out_file_table[file_id];
            }
            else if (!with_entries && file_id >= 1 && file_id <= out_file_table_vec.size()) {
                valid_id = true;
                filename = out_file_table_vec[file_id - 1];
            }

            if (valid_id == false) {
                std::cout << "Invalid file ID.\n";
                continue;
            }

            std::string file_content;
            local_client.do_get_file_content(filename, file_content, out_response);

            if (out_response != response::ok) {
                print_response_code(out_response);
                continue;
            }

            std::cout << "\nThe contents of '" << filename << "': \n";
            std::cout << file_content << std::endl;
        }
        else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_has_file() {
    std::cout << "Enter the filename you want to check for in the index: \n";

    std::string filename;
    std::getline(std::cin, filename);

    response out_response;
    bool connection_error_occured = local_client.do_has_file(filename, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nFile IS present in the server index right now.\n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_add_file() {
    std::cout << "Do you want to send and add your own local file to the server index (y - yes / n - find the file in the server storage)?: ";

    char input;
    std::cin >> input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    input = std::tolower(input);

    bool send_file = input == 'y';

    if (send_file) {
        std::cout << "Enter the filename on your local storage: \n";
    }
    else {
        std::cout << "Enter the filename in the server index: \n";
    }

    std::string filename;
    std::getline(std::cin, filename);

    std::string save_as_filename;
    std::string file_content;
    if (send_file) {
        std::u32string utf32_filename = utf_converter<char32_t>::utf8_to_string_type(filename);
        std::filesystem::path filepath = utf32_filename;

        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cout << "Failed to open the file '" << filename << "'. \n";
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string content;
        content.resize_and_overwrite(size, [&](char* buf, std::size_t) {
            file.read(buf, size);
            return static_cast<std::size_t>(file.gcount());
        });

        file_content = std::move(content);

        std::cout << "\nEnter the filename to 'Save as' on the server storage: \n";
        std::getline(std::cin, save_as_filename);
    }

    big_id_type out_write_task_id = 0;
    response out_response;

    bool connection_error_occured = false;

    if (send_file) {
        connection_error_occured = local_client.do_add_create_file(save_as_filename, file_content, out_write_task_id, out_response);
    }
    else {
        connection_error_occured = local_client.do_add_file(filename, out_write_task_id, out_response);
    }

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe ID for this 'add' task is: " << out_write_task_id << ". \n";
        std:: cout << "It will be processed later. Use 'Get Write Result' command to check the status. \n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_remove_file() {
    std::cout << "Enter the filename you want to remove from the index: \n";

    std::string filename;
    std::getline(std::cin, filename);

    big_id_type out_write_task_id = 0;
    response out_response;
    bool connection_error_occured = local_client.do_remove_file(filename, out_write_task_id, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe ID for this 'remove' task is: " << out_write_task_id << ". \n";
        std::cout << "It will be processed later. Use 'Get Write Result' command to check the status. \n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_modify_file() {
    std::cout << "Enter the filename you want to modify (update) from the index: \n";

    std::string filename;
    std::getline(std::cin, filename);

    big_id_type out_write_task_id = 0;
    response out_response;
    bool connection_error_occured = local_client.do_modify_file(filename, out_write_task_id, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe ID for this 'modify' task is: " << out_write_task_id << ". \n";
        std::cout << "It will be processed later. Use 'Get Write Result' command to check the status. \n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_get_write_result() {
    std::cout << "Enter the task ID which status you want to check: ";

    big_id_type write_task_id = get_choice<big_id_type>(false);
    response out_response;
    bool connection_error_occured = local_client.do_get_write_result(write_task_id, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response == response::write_task_id_not_found) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe status of task with the ID of " << write_task_id << " is: ";
        print_response_code(out_response);
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_get_file_content() {
    std::cout << "Enter the filename from the server which contents you want to get: \n";

    std::string filename;
    std::getline(std::cin, filename);

    std::string out_file_content;
    response out_response;
    bool connection_error_occured = local_client.do_get_file_content(filename, out_file_content, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe contents of '" << filename << "': \n";
        std::cout << out_file_content << std::endl;
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_get_reader_duration() {
    float out_reader_duration;

    response out_response;
    bool connection_error_occured = local_client.do_get_reader_duration(out_reader_duration, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::ios_base::fmtflags old_flags = std::cout.flags();
        std::streamsize old_precision = std::cout.precision();
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "\nThe reader duration on the server is " << out_reader_duration << "s \n";

        std::cout.precision(old_precision);
        std::cout.flags(old_flags);
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_get_writer_duration() {
    float out_writer_duration;

    response out_response;
    bool connection_error_occured = local_client.do_get_writer_duration(out_writer_duration, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::ios_base::fmtflags old_flags = std::cout.flags();
        std::streamsize old_prec = std::cout.precision();
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "\nThe writer duration on the server is " << out_writer_duration << "s \n";

        std::cout.precision(old_prec);
        std::cout.flags(old_flags);
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_set_new_reader_duration() {
    std::cout << "Enter the new reader duration you want to set on the server (s): ";

    float new_reader_duration = get_choice<float>(false);

    response out_response;
    bool connection_error_occured = local_client.do_set_new_reader_duration(new_reader_duration, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe new reader duration has been successfully set on the server. \n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::menu_set_new_writer_duration() {
    std::cout << "Enter the new writer duration you want to set on the server (s): ";

    float new_writer_duration = get_choice<float>(false);

    response out_response;
    bool connection_error_occured = local_client.do_set_new_writer_duration(new_writer_duration, out_response);

    if (connection_error_occured) {
        std::cout << "Connection error occured.\n";
        return;
    }

    std::cout << "\nResult: ";
    if (out_response != response::ok) {
        print_response_code(out_response);
        return;
    }
    else {
        std::cout << "\nThe new writer duration has been successfully set on the server. \n";
    }
}

template<typename string_type>
inline void program_menu<string_type>::print_response_code(response response_code) const {
    auto it = response_map.find(response_code);
    if (it != response_map.end()) {
        std::cout << it->second << "\n";
    }
    else {
        std::cout << "invalid response code\n";
    }
}
