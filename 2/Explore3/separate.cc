#include <cmath>
#include <cstdio>

int main() {
    freopen("block100.log", "r", stdin);
    freopen("block_100.log", "w", stdout);

    int a, b, c = 0, d;
    int st, score, ref_st, ref_en, lt, d1, d2;
    while (scanf("[%d]\t%d %d %d\t%d %d %d\n", &st, &score, &ref_st, &ref_en, &lt, &d1, &d2) == 7) {
        if (abs(ref_st - c) > 3) {
            puts("___________");
        }
        printf("[%5d]\t%d %d %d\n", st, score, ref_st, ref_en);

        c = ref_en;
    }
}