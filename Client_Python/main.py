from client import *

def main():
    local_client = client("127.0.0.1", 8080)

    response_map = {
        response.OK: "OK",
        response.INVALID_COMMAND: "INVALID_COMMAND",
        response.ERROR_RECEIVING_COMMAND: "ERROR_RECEIVING_COMMAND",
        response.ERROR_RECEIVING_DATA: "ERROR_RECEIVING_DATA",
        response.ARGUMENT_IS_ZERO: "ARGUMENT_IS_ZERO",
        response.SEARCH_QUERY_ENTRIES_NOT_FOUND: "SEARCH_QUERY_ENTRIES_NOT_FOUND",
        response.FILE_NOT_FOUND: "FILE_NOT_FOUND",
        response.COULD_NOT_ADD_FILE: "COULD_NOT_ADD_FILE",
        response.NEW_DURATION_IS_WAY_TOO_SMALL: "NEW_DURATION_IS_WAY_TOO_SMALL",
        response.OPERATION_IS_NOT_PROCESSED: "OPERATION_IS_NOT_PROCESSED",
        response.OPERATION_IS_IN_PROGRESS: "OPERATION_IS_IN_PROGRESS",
        response.WRITE_TASK_ID_NOT_FOUND: "WRITE_TASK_ID_NOT_FOUND"
    }

    while (True):
        print("\n1. Search")
        print("0. Exit")

        choice = input("Choose an option: ")

        if choice == 0:
            break

        print("Enter search query: ")
        search_query = input()

        words = search_query.lower().split()

        files_or_entries = input("\nAlso get words entries in files (y - yes / n - just files)?: ")

        connection_error_occured = False
        out_response = 0
        with_entries = (files_or_entries == 'y')

        out_word_entries = {}
        out_file_table = {}
        out_file_table_vec = []

        if with_entries:
            connection_error_occured, out_response = local_client.do_search(words, out_word_entries, out_file_table)
        else:
            connection_error_occured, out_response = local_client.do_search_files_only(words, out_file_table_vec)

        if connection_error_occured:
            print("Connection error occured.")

        print("\nResult: ")
        if (out_response != response.OK):
            print(response_map[out_response])
            continue

        if with_entries:
            print("\n---------------Files with IDs:\n---------------")
            for file_id, filename in out_file_table.items():
                print(f"{file_id}. {filename}")

            print("---------------------------------\nEntries (file ID, word position):\n---------------------------------")
            for entry in out_word_entries:
                print(f"({entry.file_id}, {entry.position})   ", end="")

            print()

        else:
            print("------\nFiles:\n------")
            for file_idx, filename in enumerate(out_file_table_vec):
                print(f"{file_idx}. {filename}")

            print()


if __name__ == "__main__":
    main()