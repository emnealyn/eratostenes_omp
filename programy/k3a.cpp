#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>
#include <algorithm>

int calculate(int n, int m){
    int prime_counter = 0;
    int limit = (int)sqrt(n);

    bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
    memset(result, true, (n - m + 1) * sizeof(bool));
        
    bool* primeArray = (bool*)malloc((limit + 1) * sizeof(bool));
    memset(primeArray, true, (limit + 1) * sizeof(bool));
    primeArray[0] = false;
    primeArray[1] = false;
    
	for (int i = 2; i*i*i*i <= n; i++) {
		if (primeArray[i] == true) {
			for (int j = i * i; j*j <= n; j+=i) {
				primeArray[j] = false;
			}
		}
	}

    int blockSize = 65536; 
    int numberOfBlocks = (n - m) / blockSize;
    if ((n - m) % blockSize != 0) { 
        numberOfBlocks++;
    }

    for (int i = 0; i < numberOfBlocks; i++) {
        int low = m + i * blockSize; 
        int high = low + blockSize - 1;
        if (high > n) {
            high = n;
        }

        for (int j = 2; j * j <= high; j++) {
            if (primeArray[j] == true) {
                int firstMultiple = (low / j);
                if (firstMultiple <= 1) {
                    firstMultiple = j + j;
                } else if (low % j) {
                    firstMultiple = (firstMultiple * j) + j;
                }  else {
                    firstMultiple = (firstMultiple * j);
                }
                for (int k = firstMultiple; k <= high; k += j) {
                    result[k - m] = false;
                }
            }
        }
    }
    if (m <= 0 && n >= 0) result[0 - m] = false;
    if (m <= 1 && n >= 1) result[1 - m] = false;

    for (int i = m; i <= n; i++) {
        if (result[i - m]) prime_counter++;
    }

    free(result);
    free(primeArray);
    
    return prime_counter;
}

int main(){
    int m = 2;
    int n = 1e8;
    double start_time, stop_time;

    // start_time = omp_get_wtime();
    int prime_counter = calculate(n, m);
    // stop_time = omp_get_wtime();

    printf("Liczba liczb pierwszych: %d\n", prime_counter);
    // printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);
    
    return 0;
}