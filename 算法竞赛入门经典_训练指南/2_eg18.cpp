#include <cstdio>
#include <algorithm>
#define LL long long
#define maxn 51
int main()
{
    int n, t;
    while (scanf("%d%d", &n, &t) == 2 && n)
    {
        LL pow[maxn];
        pow[0] = 1;
        for (int i = 1; i <= n; i++)
            pow[i] = pow[i - 1] * t;
        LL a = 0;
        for (int i = 0; i < n; i++)
            a += pow[std::__gcd(i, n)];
        LL b = 0;
        if (n % 2 == 1)
            b = n * pow[(n + 1) / 2];
        else
            b = n / 2 * (pow[n / 2 + 1] + pow[n / 2]);
        printf("%lld %lld\n", a / n, (a + b) / 2 / n);
    }
    return 0;
}