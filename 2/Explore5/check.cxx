#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include "../edlib.cpp"
#include "../edlib.h"

const int REF_LEN = 19407;
const int QUERY_LEN = 21638;

char Ref[REF_LEN + 100];
char query[QUERY_LEN + 100];
char query_rc[QUERY_LEN + 100];

auto calculate_distance(int ref_st, int ref_en, int query_st, int query_en) {
    const char *A = Ref + ref_st, *a = query + query_st, *_a = query_rc + (QUERY_LEN - query_en);
    int A_len = ref_en - ref_st, a_len = query_en - query_st, _a_len = query_en - query_st;

    static auto cfg = edlibDefaultAlignConfig();
    EdlibAlignResult result1 = edlibAlign(A, A_len, a, a_len, cfg);
    EdlibAlignResult result2 = edlibAlign(A, A_len, _a, _a_len, cfg);

    auto distance = std::min(result1.editDistance, result2.editDistance);

    edlibFreeAlignResult(result1);
    edlibFreeAlignResult(result2);

    return distance;
}

using point = std::tuple<int, int, int, int>;

int calculate_value(std::vector<point> points, std::string_view ref, std::string_view query, std::string_view query_rc) {
    int editdistance = 0;
    int aligned = 0;
    int preend = 0;

    std::sort(points.begin(), points.end(), [](const point& a, const point& b) { return std::get<0>(a) < std::get<0>(b); });

    for (auto [query_st, query_en, ref_st, ref_en] : points) {
        if (preend > query_st) {
            return 0;
        }
        preend = query_en;
        editdistance += calculate_distance(ref_st, ref_en, query_st, query_en);
        aligned += query_en - query_st;
    }
    return std::max(aligned - editdistance - static_cast<int>(points.size()) * 30, 0);
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

int read() {
    int x, c;
    while ((c = getchar()) < 48 || 57 < c)
        if (c == EOF)
            return -1;
    for (x = c ^ 48; 47 < (c = getchar()) && c < 58; x = x * 10 + (c ^ 48))
        ;
    return x;
}

int main() {
    std::ifstream("../reference.txt").read(Ref, REF_LEN);
    std::ifstream("../query.txt").read(query, QUERY_LEN);
    assert(strlen(Ref) == REF_LEN);
    assert(strlen(query) == QUERY_LEN);
    reverse_complement(query, query_rc, QUERY_LEN);

    std::vector<point> tuples;

    freopen("makeup.log", "r", stdin);
    while (true) {
        int a = read();
        if (a == -1)
            break;
        int b = read(), c = read(), d = read();

        tuples.emplace_back(a, b, c, d);
    }

    std::cout << calculate_value(tuples, Ref, query, query_rc) << std::endl;

    for(auto [a, b, c, d] : tuples) {
        printf("(%d, %d, %d, %d), ", a, b, c, d);
    }

    return 0;
}