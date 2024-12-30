#pragma once

#include "project_types.h"

enum class command : code_type {
    set_new_writer_duration = 245,
    set_new_reader_duration,
    get_writer_duration,
    get_reader_duration,
    get_file_content,
    get_write_result,
    modify_file,
    remove_file,
    add_file,
    has_file,
    search,
};

enum class response : code_type {
    ok = 0,
    invalid_command,
    error_receiving_command,
    error_receiving_data,
    argument_is_zero,
    search_query_entries_not_found,
    file_not_found,
    could_not_add_file,
    new_duration_is_way_too_small,
    operation_is_not_processed,
    operation_is_in_progress,
    write_task_id_not_found
};
