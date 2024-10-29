#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include "edlib.cpp"
#include "edlib.h"

std::string reverse_complement(const std::string& seq) {
    std::string rc;
    for (auto it = seq.rbegin(); it != seq.rend(); ++it) {
        switch (*it) {
            case 'A': rc += 'T'; break;
            case 'T': rc += 'A'; break;
            case 'C': rc += 'G'; break;
            case 'G': rc += 'C'; break;
        }
    }
    return rc;
}

std::vector<int> get_points(const std::string& tuples_str) {
    std::vector<int> data;
    int num = 0;
    for (char c : tuples_str) {
        if ('0' <= c && c <= '9') {
            num = num * 10 + (c - '0');
        } else if (c == ',') {
            data.push_back(num);
            num = 0;
        }
    }
    if (num != 0) {
        data.push_back(num);
    }
    return data;
}

int calculate_distance(const std::string& ref, const std::string& query, int ref_st, int ref_en, int query_st, int query_en) {
    std::string A = ref.substr(ref_st, ref_en - ref_st);
    std::string a = query.substr(query_st, query_en - query_st);
    std::string _a = reverse_complement(a);

    EdlibAlignResult result1 = edlibAlign(A.c_str(), A.size(), a.c_str(), a.size(), edlibDefaultAlignConfig());
    EdlibAlignResult result2 = edlibAlign(A.c_str(), A.size(), _a.c_str(), _a.size(), edlibDefaultAlignConfig());

    int distance = std::min(result1.editDistance, result2.editDistance);

    edlibFreeAlignResult(result1);
    edlibFreeAlignResult(result2);

    return distance;
}

int calculate_value(const std::string& tuples_str, const std::string& ref, const std::string& query) {
    try {
        std::vector<int> slicepoints = get_points(tuples_str);
        if (!slicepoints.empty() && slicepoints.size() % 4 == 0) {
            int editdistance = 0;
            int aligned = 0;
            int preend = 0;
            std::vector<std::vector<int>> points;

            for (size_t i = 0; i < slicepoints.size(); i += 4) {
                points.push_back({slicepoints[i], slicepoints[i + 1], slicepoints[i + 2], slicepoints[i + 3]});
            }

            std::sort(points.begin(), points.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
                return a[0] < b[0];
            });

            for (const auto& onetuple : points) {
                int query_st = onetuple[0];
                int query_en = onetuple[1];
                int ref_st = onetuple[2];
                int ref_en = onetuple[3];

                if (preend > query_st) {
                    return 0;
                }
                preend = query_en;
                editdistance += calculate_distance(ref, query, ref_st, ref_en, query_st, query_en);
                aligned += query_en - query_st;
            }
            return std::max(aligned - editdistance - static_cast<int>(points.size()) * 30, 0);
        } else {
            return 0;
        }
    } catch (...) {
        return 0;
    }
}

int main() {
    std::ifstream ref_file("reference.txt");
    std::ifstream query_file("query.txt");

    std::string ref((std::istreambuf_iterator<char>(ref_file)), std::istreambuf_iterator<char>());
    std::string query((std::istreambuf_iterator<char>(query_file)), std::istreambuf_iterator<char>());

    std::string tuples = "(0,100,0,95),(100,500,200,400)";

    std::cout << calculate_value(tuples, ref, query) << std::endl;

    return 0;
}