#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>
int main() {
    std::vector<std::pair<int, int>> vec;
    {
        FILE* fp = fopen("best.csv", "r");
        for (int a, b, c, d, e; fscanf(fp, "%d,%d,%d,%d,%d", &a, &b, &c, &d, &e) != EOF;) {
            assert(a + b == 29829);
            vec.emplace_back(a, b);
        }
        fclose(fp);
    }

    std::sort(vec.begin(), vec.end());

    {
        FILE* fp = fopen("answer.csv", "w");
        for (const auto& [a, b] : vec) {
            fprintf(fp, "%d,%d,%d,%d,29805\n", a, b, a, b - 1);
        }
        fclose(fp);
    }
}