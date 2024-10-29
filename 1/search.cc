#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <string>
#include <cstring>
#include "edlib.cpp" // edlib库的实现文件
#include "edlib.h" // edlib库可以用作C++的外部依赖

std::mutex mtx;  // 用于线程间同步

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

// 读取文件内容到字符串
std::string read_sequence_from_file(const std::string& file_path) {
    std::ifstream file(file_path);
    std::string sequence;
    if (file.is_open()) {
        std::getline(file, sequence);
        file.close();
    }
    return sequence;
}

// 计算编辑距离
int calculate_value(const std::string& ref, const std::string& query, int ref_st, int ref_en, int query_st, int query_en) {
    std::string A = ref.substr(0, ref_st);
    std::string a = query.substr(0, query_st);
    std::string B = ref.substr(ref_st, ref_en - ref_st);
    std::string b_rc = rc(query.substr(query_st, query_en - query_st));
    std::string C = ref.substr(ref_en);
    std::string c = query.substr(query_en);

    EdlibAlignResult resultA = edlibAlign(A.c_str(), A.size(), a.c_str(), a.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultB = edlibAlign(B.c_str(), B.size(), b_rc.c_str(), b_rc.size(), edlibDefaultAlignConfig());
    EdlibAlignResult resultC = edlibAlign(C.c_str(), C.size(), c.c_str(), c.size(), edlibDefaultAlignConfig());

    int editDistance = resultA.editDistance + resultB.editDistance + resultC.editDistance;

    edlibFreeAlignResult(resultA);
    edlibFreeAlignResult(resultB);
    edlibFreeAlignResult(resultC);

    return std::max(0, static_cast<int>(ref.size()) - editDistance);
}

// 参数结构体
struct Params {
    int ref_st;
    int ref_en;
    int query_st;
    int query_en;
};

// 优化参数
Params optimize_parameters(const std::string& ref, const std::string& query) {
    int best_value = -1;
    Params best_params = {0, 0, 0, 0};

    int step = 300;
    int ref_start_min = 5000, ref_start_max = 8000;
    int ref_end_min = 22000, ref_end_max = 25000;
    int query_start_min = 5000, query_start_max = 8000;
    int query_end_min = 22000, query_end_max = 25000;

    #pragma omp parallel for collapse(4)
    for (int ref_st = ref_start_min; ref_st < ref_start_max; ref_st += step) {
        for (int ref_en = ref_end_min; ref_en < ref_end_max; ref_en += step) {
            for (int query_st = query_start_min; query_st < query_start_max; query_st += step) {
                for (int query_en = query_end_min; query_en < query_end_max; query_en += step) {
                    int value = calculate_value(ref, query, ref_st, ref_en, query_st, query_en);
                    #pragma omp critical
                    {
                        if (value > best_value) {
                            best_value = value;
                            best_params = {ref_st, ref_en, query_st, query_en};
                        }
                    }
                }
            }
        }
    }

    return best_params;
}

// 主函数
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