#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <string>
#include <string_view>
#include "edlib.cpp" // edlib库的实现文件
#include "edlib.h" // edlib library

std::mutex mtx;  // Mutex for thread synchronization

// Reverse complement of the sequence
std::string rc(std::string_view seq) {
    std::string result(seq.rbegin(), seq.rend());
    for (char& c : result) {
        switch (c) {
            case 'A': c = 'T'; break;
            case 'T': c = 'A'; break;
            case 'C': c = 'G'; break;
            case 'G': c = 'C'; break;
        }
    }
    return result;
}

// Read file contents into a string
std::string read_sequence_from_file(const std::string& file_path) {
    std::ifstream file(file_path);
    std::string sequence;
    if (file.is_open()) {
        std::getline(file, sequence);
        file.close();
    }
    return sequence;
}

// Compute edit distance
int calculate_value(std::string_view ref, std::string_view query, int ref_st, int ref_en, int query_st, int query_en) {
    std::string_view A = ref.substr(0, ref_st);
    std::string_view a = query.substr(0, query_st);
    std::string_view B = ref.substr(ref_st, ref_en - ref_st);
    std::string b_rc = rc(query.substr(query_st, query_en - query_st));
    std::string_view C = ref.substr(ref_en);
    std::string_view c = query.substr(query_en);

    EdlibAlignResult resultA = edlibAlign(A.data(), A.size(), a.data(), a.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultB = edlibAlign(B.data(), B.size(), b_rc.data(), b_rc.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultC = edlibAlign(C.data(), C.size(), c.data(), c.size(), edlibDefaultAlignConfig());

    int editDistance = resultA.editDistance + resultB.editDistance + resultC.editDistance;

    edlibFreeAlignResult(resultA);
    edlibFreeAlignResult(resultB);
    edlibFreeAlignResult(resultC);

    return std::max(0, static_cast<int>(ref.size()) - editDistance);
}

// Parameter struct
struct Params {
    int ref_st;
    int ref_en;
    int query_st;
    int query_en;
};

// Optimizing parameters with multi-threading
void optimize_chunk(const std::string& ref, const std::string& query, int ref_start_min, int ref_start_max, int step, Params& best_params, int& best_value) {
    for (int ref_st = ref_start_min; ref_st < ref_start_max; ref_st += step) {
        for (int ref_en = 22000; ref_en < 25000; ref_en += step) {
            for (int query_st = 5000; query_st < 8000; query_st += step) {
                for (int query_en = 22000; query_en < 25000; query_en += step) {
                    int value = calculate_value(ref, query, ref_st, ref_en, query_st, query_en);
                    std::lock_guard<std::mutex> lock(mtx);
                    if (value > best_value) {
                        best_value = value;
                        best_params = {ref_st, ref_en, query_st, query_en};
                    }
                }
            }
        }
    }
}

Params optimize_parameters(const std::string& ref, const std::string& query) {
    Params best_params = {0, 0, 0, 0};
    int best_value = -1;

    int step = 50;
    std::vector<std::thread> threads;
    int num_threads = 20;  // Number of threads
    int ref_start_min = 5000, ref_start_max = 8000;

    // Divide the ref_start range among threads
    int chunk_size = (ref_start_max - ref_start_min) / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start = ref_start_min + i * chunk_size;
        int end = (i == num_threads - 1) ? ref_start_max : start + chunk_size;
        threads.emplace_back(optimize_chunk, std::ref(ref), std::ref(query), start, end, step, std::ref(best_params), std::ref(best_value));
    }

    for (auto& t : threads) {
        t.join();  // Wait for all threads to complete
    }

    return best_params;
}

// Main function
int main() {
    std::string ref = read_sequence_from_file("reference.txt");
    std::string query = read_sequence_from_file("sample.txt");

    Params best_params = optimize_parameters(ref, query);
    int best_value = calculate_value(ref, query, best_params.ref_st, best_params.ref_en, best_params.query_st, best_params.query_en);

    std::cout << "Best parameters: ref_st=" << best_params.ref_st << ", ref_en=" << best_params.ref_en
              << ", query_st=" << best_params.query_st << ", query_en=" << best_params.query_en << std::endl;
    std::cout << "Best value: " << best_value << std::endl;

    return 0;
}
