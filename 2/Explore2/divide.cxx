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

const int LIMIT = (QUERY_LEN - REF_LEN) * 1.1;

void compute_block(int st, int en, int& score) {
    for (int ref_st = std::max(0, st - LIMIT);
         ref_st < std::min(REF_LEN - (en - st), st + LIMIT);
         ++ref_st) {
        int ref_en = std::min(REF_LEN, ref_st + en - st);

        auto [dist1, dist2] = distances(ref_st, ref_en, st, en);
        int val = en - st - std::min(dist1, dist2);
        score = std::max(score, val);
    }
}
int dfs(int st, int en) {
    if (en - st < 100) {
        int score = 0;
        compute_block(st, en, score);
        return score;
    }
    int mid = (st + en) / 2;
    int d1 = dfs(st, mid);
    int d2 = dfs(mid, en);
    return d1 + d2;
}

void reverse_complement(const char* seq, char* rc, int length) {
    static char trans[256];
    trans['A'] = 'T';
    trans['T'] = 'A';
    trans['C'] = 'G';
    trans['G'] = 'C';

    for (int i = 0; i < length; ++i)
        rc[i] = trans[seq[length - 1 - i]];
    rc[length] = '\0';  // 确保字符串以空字符结尾
}

int main() {
    std::ifstream("reference.txt").read(Ref, REF_LEN);
    std::ifstream("query.txt").read(query, QUERY_LEN);
    // assert(strlen(Ref) == REF_LEN);
    // assert(strlen(query) == QUERY_LEN);
    reverse_complement(query, query_rc, QUERY_LEN);

    std::cout << dfs(0, QUERY_LEN) << std::endl;

    return 0;
}