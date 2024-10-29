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
const int step = 20;
static inline void foo(const std::string &Ref, const std::string &query, const std::string &output_file)
{
    std::ofstream out(output_file);

    build_dp_matrix(Ref, query);

        static_assert(step <= MAX_REF_LEN && step <= MAX_QUERY_LEN);
    for (int i = step; i <= Ref.size(); i += step)
    {
        out << dp[i][step];
        for (int j = 2 * step; j <= query.size(); j += step)
        {
            out << ',' << dp[i][j];
        }
        out << '\n';
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

    char filename[] = "prefix___.csv";
    filename[7] = '0' + step / 10;
    filename[8] = '0' + step % 10;
    foo(Ref, query, filename);

    // reverse(Ref.begin(), Ref.end());
    // reverse(query.begin(), query.end());

    // foo(Ref, query, "suffix.txt");
}
