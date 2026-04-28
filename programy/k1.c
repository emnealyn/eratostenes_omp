#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

int main(void){
    int m = 2;
    int n = 100000000; /* 1e8 */
    clock_t start_time, stop_time;

    size_t result_len = (size_t)(n - m + 1);
    bool *result = malloc(result_len * sizeof(bool));
    if (!result) { perror("malloc"); return 1; }
    memset(result, 1, result_len * sizeof(bool));

    int sqrt_n = (int)floor(sqrt((double)n)) + 1;
    bool *primeArray = malloc((size_t)sqrt_n * sizeof(bool));
    if (!primeArray) { perror("malloc"); free(result); return 1; }
    memset(primeArray, 1, (size_t)sqrt_n * sizeof(bool));

    start_time = clock();

    for (int i = 2; i * i <= n; i++) {
        for (int j = 2; j * j <= i; j++) {
            if (primeArray[j] && i % j == 0) {
                primeArray[i] = false;
                break;
            }
        }
    }
    for (int i = m; i <= n; i++){
        for (int j = 2; j * j <= i; j++){
            if (primeArray[j] && i % j == 0) {
                result[i - m] = false;
                break;
            }
        }
    }
    stop_time = clock();

    double elapsed = (double)(stop_time - start_time) / (double)CLOCKS_PER_SEC;
    printf("Czas trwania obliczen - wallclock %f sekund \n", elapsed);

    free(result);
    free(primeArray);
    return 0;
}
