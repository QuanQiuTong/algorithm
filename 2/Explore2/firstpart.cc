#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "edlib.cpp"
#include "edlib.h"

const int MAX_REF_LEN = 19407 + 100;
const int MAX_QUERY_LEN = 21638 + 100;

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
    std::sort(points.begin(), points.end(), [](const point &a, const point &b)
              { return std::get<0>(a) < std::get<0>(b); });

    int editdistance = 0, aligned = 0;
    int preend = 0;
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

int main()
{
    // std::ios::sync_with_stdio(false);
    std::ifstream ref_file("reference.txt");
    std::ifstream query_file("query.txt");

    auto Ref = std::string((std::istreambuf_iterator<char>(ref_file)), std::istreambuf_iterator<char>());
    auto query = std::string((std::istreambuf_iterator<char>(query_file)), std::istreambuf_iterator<char>());

    int max_value = 2293, min_dist = 1e9;
    point max_point;
    FILE *fp = fopen("firstpart3.log", "w");
    for (int i = 2420; i < 3000; ++i)
        for (int j = 2600; j < 3000; ++j)
        {
            int dist = calculate_distance(Ref, query, query, 0, i, 0, j);
            int value = j - 0 - dist;
            if (value >= max_value)
            {
                max_value = value;
                max_point = {0, i, 0, j};
                // std::cout << "i: " << i << " j: " << j << " value: " << value << std::endl;
                fprintf(fp, "i: %d j: %d value: %d dist: %d\n", i, j, value, dist);
            }
        }
    fclose(fp);

    std::cout << max_value << std::endl;
    auto [ref_st, ref_en, query_st, query_en] = max_point;
    std::cout << ref_st << " " << ref_en << " " << query_st << " " << query_en << std::endl;
}