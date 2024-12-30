#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include "server.h"
#include <chrono>
int main(int argc, char* argv[]) {
    try {
        server<string_type>::init_protocol();
        
        server<string_type> index_server;
        index_manager<string_type>& index = index_server.get_index();

        // TO-DO: initialize server_ip and server_port from command line arguments
        const std::string server_ip = "127.0.0.1";
        constexpr int server_port = 8080;

        index_server.init_server(server_ip, server_port);

        struct sockaddr_in client_address;
        int client_address_size = sizeof(client_address);
        SOCKET client_socket;

        while (client_socket = accept(index_server.get_socket(), (sockaddr*)&client_address, &client_address_size)) {
            index_server.on_client_accepted(client_socket);
        }

        server<string_type>::terminate_protocol();
    }
    catch (const std::exception& e) {
        std::cout << e.what();
    }

    //std::locale::global(std::locale(""));  // (*)
    //std::wcout.imbue(std::locale());

    return 0;
}
