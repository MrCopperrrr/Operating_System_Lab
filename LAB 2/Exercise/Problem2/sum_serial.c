#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long long sum_serial(long long n) {
    long long sum = 0;
    for (long long i = 1; i <= n; i++) {
        sum += i;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <n>\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);

    clock_t start = clock();
    long long result = sum_serial(n);
    clock_t end = clock();

    printf("Sum (Serial): %lld\n", result);
    printf("Time taken: %.6f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}
