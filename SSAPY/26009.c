#include <stdio.h>

#define SUM 998244353 

int main () {
    int n;
    long long a, b, c, result = 0;

    scanf("%d", &n);

    for(int i1 = 0; i1 < n; i1++){
        int i, j, k;
        result = 0;
        
        scanf("%d %d %d", &a, &b, &c);

        long long sumA = (a * (a + 1) / 2) % SUM;
        long long sumB = (b * (b + 1) / 2) % SUM;
        long long sumC = (c * (c + 1) / 2) % SUM;

        long long result = (sumA * sumB) % SUM;
        result = (result * sumC) % SUM;

        printf("%lld\n", result);
    }   

    

    return 0;
}