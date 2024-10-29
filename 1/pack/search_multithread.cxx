#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include "edlib.cpp"  // edlib库的实现文件
#include "edlib.h"    // edlib library

std::mutex mtx;  // Mutex for thread synchronization

constexpr int LEN = 30000, RLEN = 29830, SLEN = 29845;
char rr[LEN], ss[LEN], rcs[LEN];

// Compute edit distance
int calculate_value(int ref_st, int ref_en, int query_st, int query_en) {
    auto cfg = edlibDefaultAlignConfig();
    auto resultA = edlibAlign(rr, ref_st, ss, query_st, cfg),
         resultB = edlibAlign(rr + ref_st, ref_en - ref_st, rcs + SLEN - query_en, query_en - query_st, cfg),
         resultC = edlibAlign(rr + ref_en, RLEN - ref_en, ss + query_en, SLEN - query_en, cfg);

    int editDistance = resultA.editDistance + resultB.editDistance + resultC.editDistance;

    edlibFreeAlignResult(resultA);
    edlibFreeAlignResult(resultB);
    edlibFreeAlignResult(resultC);

    return std::max(0, RLEN - editDistance);
}

// Parameter struct
struct Params {
    int ref_st;
    int ref_en;
    int query_st;
    int query_en;
};

// Optimizing parameters with multi-threading
void optimize_chunk(int ref_start_min, int ref_start_max, int step, Params& best_params, int& best_value) {
    Params local_params;
    int local_value = -1;

#pragma omp parallel for collapse(4) schedule(dynamic) num_threads(20)
    for (int ref_st = ref_start_min; ref_st < ref_start_max; ref_st += step) {
        for (int ref_en = 22800; ref_en < 23800; ref_en += step) {
            for (int query_st = 6100; query_st < 7100; query_st += step) {
                for (int query_en = 22800; query_en < 23800; query_en += step) {
                    int value = calculate_value(ref_st, ref_en, query_st, query_en);
                    if (value > local_value) {
                        local_value = value;
                        local_params = {ref_st, ref_en, query_st, query_en};
                    }
                }
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (local_value > best_value) {
            best_value = local_value;
            best_params = local_params;
        }
    }
}

constexpr int step = 40;

auto optimize_parameters() {
    Params best_params = {0, 0, 0, 0};
    int best_value = -1;

    std::vector<std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();  // Number of threads
    int ref_start_min = 6100, ref_start_max = 7100;

    // Divide the ref_start range among threads
    int chunk_size = (ref_start_max - ref_start_min) / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start = ref_start_min + i * chunk_size;
        int end = (i == num_threads - 1) ? ref_start_max : start + chunk_size;
        threads.emplace_back(optimize_chunk, start, end, step, std::ref(best_params), std::ref(best_value));
    }

    for (auto& t : threads) {
        t.join();  // Wait for all threads to complete
    }

    return std::make_pair(best_params, best_value);
}

// Reverse complement of the sequence
std::string rc(std::string_view seq) {
    std::string result(seq.rbegin(), seq.rend());
    for (char& c : result) {
        switch (c) {
            case 'A':
                c = 'T';
                break;
            case 'T':
                c = 'A';
                break;
            case 'C':
                c = 'G';
                break;
            case 'G':
                c = 'C';
                break;
        }
    }
    return result;
}

size_t readfile(char* buffer, size_t len, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        throw std::runtime_error("Cannot open file");
    size_t read = fread(buffer, 1, len, file);
    fclose(file);

    buffer[read] = '\0';
    return read;
}

// Main function
int main() {
    // std::cout << std::thread::hardware_concurrency() << std::endl;
    readfile(rr, LEN, "reference.txt");
    size_t slen = readfile(ss, LEN, "sample.txt");

    std::string rcq = rc(std::string_view(ss, slen));
    strcpy(rcs, rcq.c_str());

    auto [best_params, best_value] = optimize_parameters();
    auto [ref_st, ref_en, query_st, query_en] = best_params;

    std::ofstream fout("output.log", std::ios_base::app);
    fout << "Step: " << step << std::endl;
    fout << "Best parameters: ref_st=" << ref_st << ", ref_en=" << ref_en
         << ", query_st=" << query_st << ", query_en=" << query_en << std::endl;
    fout << "Best value: " << best_value << std::endl;
    fout.close();

    return 0;
}
