#pragma once

#include <thread>
#include <atomic>
#include <random>
#include <limits>
#include <numeric>
#include "minimal_client.h"

template <typename string_type>
class stress_tester {
public:
    inline stress_tester(const std::string& ip_address, const int port, const std::size_t clients_amount, const std::size_t iterations);
    inline ~stress_tester() = default;

    inline stress_tester(const stress_tester& other) = delete;
    inline stress_tester(stress_tester&& other) = delete;
    inline stress_tester& operator=(const stress_tester& other) = delete;
    inline stress_tester& operator=(stress_tester&& other) = delete;

public:
    void start_test();

private:
    void test_client_function(minimal_client<string_type>& client, const string_type& word);

    std::vector<minimal_client<string_type>> clients;
    std::vector<std::thread> threads;
    std::size_t clients_amount;
    std::size_t iterations;

    std::atomic<bool> start_flag = false;
};

template<typename string_type>
inline stress_tester<string_type>::stress_tester(const std::string& ip_address, const int port, const std::size_t clients_amount, const std::size_t iterations) {
    this->clients_amount = clients_amount;
    this->iterations = iterations;
    
    threads.resize(clients_amount);
    clients.reserve(clients_amount);

    for (std::size_t i = 0; i < clients_amount; ++i) {
        clients.emplace_back(ip_address, port);
    }
}

template<typename string_type>
inline void stress_tester<string_type>::start_test() {
    std::vector<std::string> words = { "demon", "hi", "best", "worst", "actor", "pay", "decided", "another", };

    using char_type = string_type::value_type;

    std::vector<string_type> wide_words;
    wide_words.reserve(words.size());

    for (const auto& word : words) {
        wide_words.emplace_back(utf_converter<char_type>::utf8_to_string_type(word));
    }

    std::random_device rand_device;
    std::mt19937 rand_gen{ rand_device() };
    std::uniform_int_distribution<std::size_t> uni_dist(std::numeric_limits<std::size_t>::min(), std::numeric_limits<std::size_t>::max());

    std::vector<long long> sum_of_times(4, 0);

    for (std::size_t iter = 0; iter < iterations; ++iter) {
        start_flag.store(false, std::memory_order::memory_order_release);

        //auto start1 = std::chrono::high_resolution_clock::now();
        for (std::size_t i = 0; i < clients_amount; ++i) {
            const auto& wide_word = wide_words[uni_dist(rand_gen) % wide_words.size()];
            threads[i] = std::thread(&stress_tester<string_type>::test_client_function, this, std::ref(clients[i]), std::cref(wide_word));
        }
        //auto end1 = std::chrono::high_resolution_clock::now();
        //auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1);
        std::cout << "Creating threads time: \n";// << time1.count() << " ns\n";

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(300ms);

        start_flag.store(true, std::memory_order::memory_order_release);

        for (std::size_t i = 0; i < clients_amount; ++i) {
            threads[i].join();
        }

        long long queue_waiting_time = 0;
        long long getting_query_time = 0;
        long long query_processing_time = 0;
        long long sending_results_time = 0;

        for (std::size_t i = 0; i < clients_amount; ++i) {
            queue_waiting_time += clients[i].get_measurements()[1 - 1];
            getting_query_time += clients[i].get_measurements()[2 - 1];
            query_processing_time += clients[i].get_measurements()[3 - 1];
            sending_results_time += clients[i].get_measurements()[4 - 1];
        }

        queue_waiting_time /= clients_amount;
        getting_query_time /= clients_amount;
        query_processing_time /= clients_amount;
        sending_results_time /= clients_amount;

        sum_of_times[0] += queue_waiting_time;
        sum_of_times[1] += getting_query_time;
        sum_of_times[2] += query_processing_time;
        sum_of_times[3] += sending_results_time;
    }

    std::cout << "\n=== Stress Test Results ===\n\n";
    std::cout << "Amount of client threads: " << clients_amount << "\n\n";
    std::cout << "queue waiting time:    " << sum_of_times[0] / iterations << " ns\n";
    std::cout << "getting query time:    " << sum_of_times[1] / iterations << " ns\n";
    std::cout << "query processing time: " << sum_of_times[2] / iterations << " ns\n";
    std::cout << "sending results time:  " << sum_of_times[3] / iterations << " ns\n";

    long long total_time = std::accumulate(sum_of_times.begin(), sum_of_times.end(), 0ll);
    std::cout << "\nTOTAL time:            " << total_time / iterations << " ns\n";

    // Don't forget to uncomment send_responce_code in server class for this test!
}

template<typename string_type>
inline void stress_tester<string_type>::test_client_function(minimal_client<string_type>& client, const string_type& word) {
    std::unordered_set<string_type> word_set = { word };
    std::vector<std::string> out_file_table;
    response out_response;

    while (!start_flag.load(std::memory_order_acquire)) {
        // Busy wait. It would be fair to perform all the queries in a single moment.
    }
    client.do_search_files_only(word_set, out_file_table, out_response);
}
