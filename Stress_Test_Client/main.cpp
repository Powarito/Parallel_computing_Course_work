#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include "minimal_client.h"
#include "stress_tester.h"

int main(int argc, char* argv[]) {
    try {
        minimal_client<string_type>::init_protocol();

        std::size_t clients_amount = 1;
        std::size_t iterations = 10;
        stress_tester<string_type> tester("127.0.0.1", 8080, clients_amount, iterations);
        tester.start_test();

        int a;
        std::cin >> a;

        minimal_client<string_type>::terminate_protocol();
    }
    catch (std::exception& e) {
        std::cout << e.what();
    }

    return 0;
}
