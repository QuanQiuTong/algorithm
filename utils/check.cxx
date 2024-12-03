#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <string>
#include <memory>

#include "../1/edlib.cpp"
#include "../1/edlib.h"

using point = std::tuple<int, int, int, int>;

auto calculate_distance(const char* Ref, const char* query, const char* query_rc, int QUERY_LEN, const point& p) {
    auto [query_st, query_en, ref_st, ref_en] = p;

    const char* A = Ref + ref_st;
    const char* a = query + query_st;
    const char* _a = query_rc + (QUERY_LEN - query_en);
    int A_len = ref_en - ref_st;
    int a_len = query_en - query_st;
    int _a_len = query_en - query_st;

    static auto cfg = edlibDefaultAlignConfig();
    EdlibAlignResult result1 = edlibAlign(A, A_len, a, a_len, cfg);
    EdlibAlignResult result2 = edlibAlign(A, A_len, _a, _a_len, cfg);

    auto distance = std::min(result1.editDistance, result2.editDistance);

    edlibFreeAlignResult(result1);
    edlibFreeAlignResult(result2);

    return distance;
}

int calculate_value(const std::vector<point>& points, const char* Ref, const char* query, const char* query_rc, int QUERY_LEN) {
    int editdistance = 0;
    int aligned = 0;
    int preend = 0;

    std::vector<point> sorted_points = points;
    std::sort(sorted_points.begin(), sorted_points.end(), [](const point& a, const point& b) { return std::get<0>(a) < std::get<0>(b); });

    for (const auto& p : sorted_points) {
        int query_st = std::get<0>(p);
        int query_en = std::get<1>(p);
        if (preend > query_st) {
            return 0;
        }
        preend = query_en;
        editdistance += calculate_distance(Ref, query, query_rc, QUERY_LEN, p);
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
        rc[i] = trans[static_cast<unsigned char>(seq[length - 1 - i])];
    rc[length] = '\0';
}

int read_number() {
    int x, c;
    while ((c = getchar()) < 48 || 57 < c)
        if (c == EOF)
            return -1;
    for (x = c ^ 48; 47 < (c = getchar()) && c < 58; x = x * 10 + (c ^ 48))
        ;
    return x;
}

bool read_file(const std::string& filename, std::unique_ptr<char[]>& buffer, int& length) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = std::make_unique<char[]>(length + 1);
    if (!file.read(buffer.get(), length)) {
        std::cerr << "Failed to read file: " << filename << std::endl;
        return false;
    }
    buffer[length] = '\0';

    return true;
}

int main() {
    std::unique_ptr<char[]> Ref;
    std::unique_ptr<char[]> query;
    std::unique_ptr<char[]> query_rc;
    int REF_LEN = 0;
    int QUERY_LEN = 0;

    if (!read_file("../chr/chr6.fasta", Ref, REF_LEN)) {
        return 1;
    }

    if (!read_file("../query.txt", query, QUERY_LEN)) {
        return 1;
    }

    query_rc = std::make_unique<char[]>(QUERY_LEN + 1);
    reverse_complement(query.get(), query_rc.get(), QUERY_LEN);

    std::vector<point> tuples;

    freopen("makeup.log", "r", stdin);
    while (true) {
        int a = read_number();
        if (a == -1)
            break;
        int b = read_number(), c = read_number(), d = read_number();

        tuples.emplace_back(a, b, c, d);
    }

    std::cout << calculate_value(tuples, Ref.get(), query.get(), query_rc.get(), QUERY_LEN) << std::endl;

    for (const auto& [a, b, c, d] : tuples) {
        printf("(%d, %d, %d, %d), ", a, b, c, d);
    }

    return 0;
}