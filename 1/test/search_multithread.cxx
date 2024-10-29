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
#include "edlib.h"   // edlib library

std::mutex mtx; // Mutex for thread synchronization

constexpr int LEN = 30000, RLEN = 64, SLEN = 64, RBEGIN = 0, REND = 64, SBEGIN = 0, SEND = 64;
char rr[LEN], ss[LEN], rcs[LEN];

// Compute edit distance
int calculate_value(int ref_st, int ref_en, int query_st, int query_en)
{
    auto A = std::string_view(rr, ref_st);
    auto a = std::string_view(ss, query_st);

    auto B = std::string_view(rr + ref_st, ref_en - ref_st);
    auto b_rc = std::string_view(rcs + SLEN - query_en, query_en - query_st);

    auto C = std::string_view(rr + ref_en, RLEN - ref_en);
    auto c = std::string_view(ss + query_en, SLEN - query_en);

    EdlibAlignResult resultA = edlibAlign(A.data(), A.size(), a.data(), a.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultB = edlibAlign(B.data(), B.size(), b_rc.data(), b_rc.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultC = edlibAlign(C.data(), C.size(), c.data(), c.size(), edlibDefaultAlignConfig());

    int editDistance = resultA.editDistance + resultB.editDistance + resultC.editDistance;

    edlibFreeAlignResult(resultA);
    edlibFreeAlignResult(resultB);
    edlibFreeAlignResult(resultC);

    return std::max(0, RLEN - editDistance);
}

// Parameter struct
struct Params
{
    int ref_st;
    int ref_en;
    int query_st;
    int query_en;
};

// Optimizing parameters with multi-threading
void optimize_chunk(int ref_start_min, int ref_start_max, int step, Params &best_params, int &best_value)
{
    for (int ref_st = ref_start_min; ref_st < ref_start_max; ref_st += step)
    {
        for (int ref_en = RBEGIN; ref_en < REND; ref_en += step)
        {
            for (int query_st = SBEGIN; query_st < SEND; query_st += step)
            {
                for (int query_en = RBEGIN; query_en < REND; query_en += step)
                {
                    int value = calculate_value(ref_st, ref_en, query_st, query_en);
                    std::lock_guard<std::mutex> lock(mtx);
                    if (value > best_value)
                    {
                        best_value = value;
                        best_params = {ref_st, ref_en, query_st, query_en};
                    }
                }
            }
        }
    }
}

Params optimize_parameters()
{
    Params best_params = {0, 0, 0, 0};
    int best_value = -1;

    int step = 30;
    std::vector<std::thread> threads;
    int num_threads = 20; // Number of threads
    int ref_start_min = SBEGIN, ref_start_max = SEND;

    // Divide the ref_start range among threads
    int chunk_size = (ref_start_max - ref_start_min) / num_threads;

    for (int i = 0; i < num_threads; ++i)
    {
        int start = ref_start_min + i * chunk_size;
        int end = (i == num_threads - 1) ? ref_start_max : start + chunk_size;
        threads.emplace_back(optimize_chunk, start, end, step, std::ref(best_params), std::ref(best_value));
    }

    for (auto &t : threads)
    {
        t.join(); // Wait for all threads to complete
    }

    return best_params;
}

const char *readfile(const char *buffer, size_t len, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return NULL;
    }
    size_t read = fread((void *)buffer, 1, len, file);
    fclose(file);
    return buffer;
}

const char *rc(const char *s, size_t len, char *rc)
{
    for (size_t i = 0; i < len; ++i)
    {
        switch (s[i])
        {
        case 'A':
            rc[len - i - 1] = 'T';
            break;
        case 'C':
            rc[len - i - 1] = 'G';
            break;
        case 'G':
            rc[len - i - 1] = 'C';
            break;
        case 'T':
            rc[len - i - 1] = 'A';
            break;
            // default: rc[len - i - 1] = s[i]; break;
        }
    }
    return rc;
}

// Main function
int main()
{
    readfile(rr, RLEN, "reference.txt");
    readfile(ss, SLEN, "sample.txt");
    rc(ss, SLEN, rcs);

    printf("reference: %s\n", rr);
    printf("query: %s\n", ss);
    printf("rc query: %s\n", rcs);

    // cout << "lengths: " << ref.size() << " " << query.size() << " " << rcq.size() << endl;

    Params best_params = optimize_parameters();
    int best_value = calculate_value(best_params.ref_st, best_params.ref_en, best_params.query_st, best_params.query_en);

    std::cout << "Best parameters: ref_st=" << best_params.ref_st << ", ref_en=" << best_params.ref_en
              << ", query_st=" << best_params.query_st << ", query_en=" << best_params.query_en << std::endl;
    std::cout << "Best value: " << best_value << std::endl;

    return 0;
}
