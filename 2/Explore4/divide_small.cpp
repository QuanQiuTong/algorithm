#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>

#include "edlib.cpp"
#include "edlib.h"

const int REF_LEN = 19407;
const int QUERY_LEN = 21638;

char Ref[REF_LEN + 1];
char query[QUERY_LEN + 1];
char query_rc[QUERY_LEN + 1];

auto edist(const char* s1, int len1, const char* s2, int len2) {
    std::vector<int> prev(len2 + 1), curr(len2 + 1);  // 每个线程需要有自己的数组

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

const int bsz = 40, bcnt = QUERY_LEN / bsz + !!(QUERY_LEN % bsz);
const int LEFT = (QUERY_LEN - REF_LEN) * 1.01, RIGHT = 210;
const int err = 40;

std::mutex mtx;

struct DivideBlock {
    int score, ref_st, ref_en, d1, d2;
} blocks[bcnt];

void compute_block(int i, int& score) {
    int query_st = i * bsz, query_en = std::min(QUERY_LEN, (i + 1) * bsz), len = query_en - query_st;
    DivideBlock& bl = blocks[i];

    for (int ref_st = std::max(0, query_st - LEFT);
         ref_st < std::min(REF_LEN - len, query_st + RIGHT);
         ++ref_st) {
        int calc_0 = std::max(0, ref_st + len - err);
        if (calc_0 < ref_st)
            calc_0 = ref_st;
        int calc_en = std::min(REF_LEN, ref_st + len + err);
        auto v1 = edist(query + query_st, query_en - query_st, Ref + ref_st, calc_en - ref_st);

        for (int ref_en = calc_0; ref_en < calc_en; ++ref_en) {
            // auto [dist1, dist2] = distances(ref_st, ref_en, query_st, query_en);
            int val = len - v1[ref_en - ref_st];
            if (val > bl.score) {
                bl.score = val;
                bl.ref_st = ref_st;
                bl.ref_en = ref_en;
                // bl.d1 = dist1;
                // bl.d2 = dist2;
            }
        }
        if(ref_st % 1000 == 0) {
            std::cout << i << ' ' << ref_st << '\n';
        }
    }

    std::lock_guard<std::mutex> lock(mtx);  // 确保对共享资源的访问是线程安全的
    score += bl.score;
}

int block() {
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

    return score;
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
    assert(strlen(Ref) == REF_LEN);
    assert(strlen(query) == QUERY_LEN);
    reverse_complement(query, query_rc, QUERY_LEN);

    int score = block();

    std::ofstream fout("block60.log");
    int i = 0, las = 0;
    for (auto [score, ref_st, ref_en, d1, d2] : blocks) {
        if (abs(ref_st - las) > 3) {
            fout << "___________" << std::endl;
        }
        las = ref_en;
        fout << '(' << (i++) * bsz << ", " << (i * bsz)
             << ", " << ref_st << ", " << ref_en << ')'
             << '\t' << score
             //  << '\t' << (d1 < d2) << ' ' << d1 << ' ' << d2
             << std::endl;
    }

    fout << " [ " << bcnt << " ] " << score << " - " << bcnt * 30 << " = " << score - bcnt * 30 << std::endl;

    return 0;
}