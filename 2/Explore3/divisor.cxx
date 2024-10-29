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

void edit_distance(const char* s1, int len1, const char* s2, int len2){
    for (int i = 0; i <= len1; ++i)
        dp[i][0] = i;
    for (int j = 0; j <= len2; ++j)
        dp[0][j] = j;

    for (int i = 1; i <= len1; ++i)
    {
        for (int j = 1; j <= len2; ++j)
        {
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1]; // Characters match
            else
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + 1}); // Min of insert, delete, replace
        }
    }
}

const int bsz = 2400, bcnt = QUERY_LEN / bsz + !!(QUERY_LEN % bsz);
const int LIMIT = (QUERY_LEN - REF_LEN) * 1.5;
const int err = 50;
std::mutex mtx;

struct DivideBlock {
    int score, ref_st, ref_en, d1, d2;
} blocks[bcnt];

void compute_block(int i, int& score) {
    int query_st = i * bsz, query_en = std::min(QUERY_LEN, (i + 1) * bsz), len = query_en - query_st;
    DivideBlock& bl = blocks[i];

    for (int ref_st = std::max(0, query_st - LIMIT);
         ref_st < std::min(REF_LEN - len, query_st + LIMIT);
         ++ref_st) {

        // Show progress
        std::cout << '[' << ref_st << ' ' << query_st << ']' << '\n';

        int calc_0 = std::max(0, ref_st + len - err);
        if(calc_0 < ref_st)
            calc_0 = ref_st;
        int calc_en = std::min(REF_LEN, ref_st + len + err);
        edit_distance(Ref + ref_st, calc_en - ref_st, query + query_st, query_en - query_st);

        for(int ref_en = calc_0; ref_en < calc_en; ++ref_en) {
            // auto [dist1, dist2] = distances(ref_st, ref_en, query_st, query_en);
            int val = len - dp[ref_en - ref_st][query_en - query_st];
            if (val > bl.score) {
                bl.score = val;
                bl.ref_st = ref_st;
                bl.ref_en = ref_en;
                // bl.d1 = dist1;
                // bl.d2 = dist2;
            }
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    score += bl.score;
}

void block() {
    int score = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < bcnt; ++i) {
        if (threads.size() >= std::thread::hardware_concurrency()) {
            for (auto& t : threads) {
                if (t.joinable())
                    t.join();
            }
            threads.clear();
        }
        threads.emplace_back(compute_block, i, std::ref(score));
    }

    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }

    std::ofstream fout("block2400.log");
    for (int i = 0; auto [score, ref_st, ref_en, d1, d2] : blocks) {
        fout << '[' << (i++) * bsz << ']'
                  << '\t' << score << ' ' << ref_st << ' ' << ref_en
                  << '\t' << (d1 < d2) << ' ' << d1 << ' ' << d2 << std::endl;
    }

    FILE* fp = fopen("tuple2400.log", "w");
    fprintf(fp, "(%d, %d, %d, %d)", 0, bsz, blocks[0].ref_st, blocks[0].ref_en);
    for (int i = 1; i < bcnt; ++i) {
        fprintf(fp, ", (%d, %d, %d, %d)", i * bsz, std::min(QUERY_LEN, (i + 1) * bsz), blocks[i].ref_st, blocks[i].ref_en);
    }
    fclose(fp);

    std::cout << " [ " << bcnt << " ] ";
    std::cout << score << " - " << bcnt * 30 << " = " << score - bcnt * 30 << std::endl;
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
    ios::sync_with_stdio(false);
    std::ifstream("reference.txt").read(Ref, REF_LEN);
    std::ifstream("query.txt").read(query, QUERY_LEN);
    // assert(strlen(Ref) == REF_LEN);
    // assert(strlen(query) == QUERY_LEN);
    reverse_complement(query, query_rc, QUERY_LEN);

    block();

    return 0;
}