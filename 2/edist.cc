#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "edlib.cpp"
#include "edlib.h"

const int MAX_REF_LEN = 19407 + 100;
const int MAX_QUERY_LEN = 21638 + 100;

int dp[MAX_REF_LEN + 1][MAX_QUERY_LEN + 1];

// Function to build the DP matrix for edit distance
void build_dp_matrix(const std::string &ref, const std::string &query)
{
    int n = ref.size();
    int m = query.size();

    for (int i = 0; i <= n; ++i)
        dp[i][0] = i;
    for (int j = 0; j <= m; ++j)
        dp[0][j] = j;

    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= m; ++j)
        {
            if (ref[i - 1] == query[j - 1])
                dp[i][j] = dp[i - 1][j - 1]; // Characters match
            else
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + 1}); // Min of insert, delete, replace
        }
    }
}

static inline void foo(const std::string &Ref, const std::string &query, const std::string &output_file)
{
    std::ofstream out(output_file);

    build_dp_matrix(Ref, query);
    int len = std::min(Ref.size(), query.size());
    for (int i = 1; i <= len; ++i)
    {
        out << dp[i][i] << '\n';
    }
    out.close();
}

int main()
{
    std::ios::sync_with_stdio(false);
    std::ifstream ref_file("reference.txt");
    std::ifstream query_file("query.txt");

    auto Ref = std::string((std::istreambuf_iterator<char>(ref_file)), std::istreambuf_iterator<char>());
    auto query = std::string((std::istreambuf_iterator<char>(query_file)), std::istreambuf_iterator<char>());

    foo(Ref, query, "prefix.txt");

    reverse(Ref.begin(), Ref.end());
    reverse(query.begin(), query.end());

    foo(Ref, query, "suffix.txt");

}

// Tests:

//

//

//

//

// Function to calculate the edit distance between any substrings using the DP matrix
int get_substring_edit_distance(int ref_st, int ref_en, int query_st, int query_en)
{
    return dp[ref_en][query_en] - dp[ref_st][query_st]; // actually, st must be 0.
}

// Function to calculate the edit distance using edlib
int calculate_edit_distance_edlib(const std::string &ref, const std::string &query)
{
    EdlibAlignResult result = edlibAlign(ref.c_str(), ref.size(), query.c_str(), query.size(), edlibDefaultAlignConfig());
    int distance = result.editDistance;
    edlibFreeAlignResult(result);
    return distance;
}

int main0()
{
    std::string ref = "AGCTAGCAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGC";
    std::string query = "GCTAGCTAGCTAGCTAGCTAGCTAGCTXXAGCTAGCTAGCTAGCTAGCTAGCTAGCTAGCTAG";

    // Build the DP matrix
    build_dp_matrix(ref, query);

    // Example usage: Get the edit distance for ref[0:50] and query[0:50]
    for (int i = 1; i <= 62; ++i)
        for (int j = 1; j <= 62; ++j)
        {
            int ref_st = 0, ref_en = i;
            int query_st = 0, query_en = j;
            int distance_dp = get_substring_edit_distance(ref_st, ref_en, query_st, query_en);

            // Calculate the edit distance using edlib
            int distance_edlib = calculate_edit_distance_edlib(ref.substr(ref_st, ref_en - ref_st), query.substr(query_st, query_en - query_st));

            if (distance_dp != distance_edlib)
            {
                std::cout << "Edit distance between ref[" << ref_st << ":" << ref_en << "] and query[" << query_st << ":" << query_en << "] using DP is " << distance_dp << std::endl;
                std::cout << "Edit distance between ref[" << ref_st << ":" << ref_en << "] and query[" << query_st << ":" << query_en << "] using edlib is " << distance_edlib << std::endl;
            }
        }
    return 0;
}