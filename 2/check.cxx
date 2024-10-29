#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include <string_view>
#include <algorithm>
#include "edlib.cpp"
#include "edlib.h"

int calculate_distance(std::string_view ref, std::string_view query, std::string_view query_rc, int ref_st, int ref_en, int query_st, int query_en)
{
    std::string_view A = ref.substr(ref_st, ref_en - ref_st);
    std::string_view a = query.substr(query_st, query_en - query_st);
    std::string_view _a = query_rc.substr(query.size() - query_en, query_en - query_st);

    EdlibAlignResult result1 = edlibAlign(A.data(), A.size(), a.data(), a.size(), edlibDefaultAlignConfig());
    EdlibAlignResult result2 = edlibAlign(A.data(), A.size(), _a.data(), _a.size(), edlibDefaultAlignConfig());

    int distance = std::min(result1.editDistance, result2.editDistance);

    edlibFreeAlignResult(result1);
    edlibFreeAlignResult(result2);

    return distance;
}

using point = std::tuple<int, int, int, int>;

int calculate_value(std::vector<point> points, std::string_view ref, std::string_view query, std::string_view query_rc)
{
    int editdistance = 0;
    int aligned = 0;
    int preend = 0;

    std::sort(points.begin(), points.end(), [](const point &a, const point &b)
              { return std::get<0>(a) < std::get<0>(b); });

    for (auto [query_st, query_en, ref_st, ref_en] : points)
    {
        if (preend > query_st)
        {
            return 0;
        }
        preend = query_en;
        editdistance += calculate_distance(ref, query, query_rc, ref_st, ref_en, query_st, query_en);
        aligned += query_en - query_st;
    }
    return std::max(aligned - editdistance - static_cast<int>(points.size()) * 30, 0);
}

std::string reverse_complement(std::string_view seq)
{
    std::string rc;
    for (auto it = seq.rbegin(); it != seq.rend(); ++it)
    {
        switch (*it)
        {
        case 'A':
            rc += 'T';
            break;
        case 'T':
            rc += 'A';
            break;
        case 'C':
            rc += 'G';
            break;
        case 'G':
            rc += 'C';
            break;
        }
    }
    return rc;
}

std::string Ref;
std::string query;
std::string query_rc;
int main()
{
    std::ifstream ref_file("reference.txt");
    std::ifstream query_file("query.txt");

    Ref = std::string((std::istreambuf_iterator<char>(ref_file)), std::istreambuf_iterator<char>());
    query = std::string((std::istreambuf_iterator<char>(query_file)), std::istreambuf_iterator<char>());

    // 计算 query 的 reverse complement
    query_rc = reverse_complement(query);

    std::vector<point> tuples({{0, 100, 0, 95}, {100, 500, 200, 400}});

    std::cout << calculate_value(tuples, Ref, query, query_rc) << std::endl;

    return 0;
}