#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include "client.h"
#include "menu.h"

int main(int argc, char* argv[]) {
    try {
        client<string_type>::init_protocol();

        client<string_type> local_client("127.0.0.1", 8080);

        program_menu<string_type> menu(local_client);
        menu.open_menu();

        client<string_type>::terminate_protocol();
    }
    catch (std::exception& e) {
        std::cout << e.what();
    }

    return 0;
}
