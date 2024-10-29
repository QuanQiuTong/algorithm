#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include "edlib.cpp"
#include "edlib.h"

const int REF_LEN = 19407;
const int QUERY_LEN = 21638;

char Ref[REF_LEN + 100];
char query[QUERY_LEN + 100];
char query_rc[QUERY_LEN + 100];

auto distances(int ref_st, int ref_en, int query_st, int query_en) {
    const char *A = Ref + ref_st, *a = query + query_st, *_a = query_rc + (QUERY_LEN - query_en);
    int A_len = ref_en - ref_st, a_len = query_en - query_st, _a_len = query_en - query_st;

    static auto cfg = edlibDefaultAlignConfig();
    EdlibAlignResult result1 = edlibAlign(A, A_len, a, a_len, cfg);
    EdlibAlignResult result2 = edlibAlign(A, A_len, _a, _a_len, cfg);

    auto distance = std::pair(result1.editDistance, result2.editDistance);

    edlibFreeAlignResult(result1);
    edlibFreeAlignResult(result2);

    return distance;
}

int dp[REF_LEN + 1][QUERY_LEN + 1];

int edit_distance(const char* s1, int len1, const char* s2, int len2) {
    for (int i = 0; i <= len1; ++i)
        dp[i][0] = i;
    for (int j = 0; j <= len2; ++j)
        dp[0][j] = j;

    for (int i = 1; i <= len1; ++i)
        for (int j = 1; j <= len2; ++j)
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];  // Characters match
            else
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + 1});  // Min of insert, delete, replace

    return dp[len1][len2];
}

auto edist(const char* s1, int len1, const char* s2, int len2) {
    std::vector<int> prev(len2 + 1), curr(len2 + 1);

    // 初始化第一行
    for (int j = 0; j <= len2; ++j)
        prev[j] = j;

    for (int i = 1; i <= len1; ++i) {
        curr[0] = i;  // 初始化当前行的第一列
        for (int j = 1; j <= len2; ++j) {
            if (s1[i - 1] == s2[j - 1])
                curr[j] = prev[j - 1];  // Characters match
            else
                curr[j] = std::min({prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + 1});  // Min of insert, delete, replace
        }
        std::swap(prev, curr);  // 交换当前行和上一行
    }

    return prev;
}

int main() {
    std::ifstream("reference.txt").read(Ref, REF_LEN);
    std::ifstream("query.txt").read(query, QUERY_LEN);
    // assert(strlen(Ref) == REF_LEN);
    // assert(strlen(query) == QUERY_LEN);
    // reverse_complement(query, query_rc, QUERY_LEN);

    // 随便找几个子串测试两个函数是否一致

    auto [ref_st, ref_en, query_st, query_en] =
        std::tuple{16923, 18961, 19600, 21638};

    int d1 = distances(ref_st, ref_en, query_st, query_en).first;
    int d2 = edit_distance(Ref + ref_st, ref_en - ref_st, query + query_st, query_en - query_st);

    std::cout << d1 << ' ' << d2 << std::endl;

    int d3 = distances(ref_st, ref_en - 200, query_st, query_en).first;
    int d4 = dp[ref_en - ref_st - 200][query_en - query_st];

    std::cout << d3 << ' ' << d4 << std::endl;

    auto v1 = edist(query + query_st, query_en - query_st, Ref + ref_st, ref_en - ref_st);

    std::cout << v1[ref_en - ref_st] << ' ' << v1[ref_en - ref_st - 200] << std::endl;
}