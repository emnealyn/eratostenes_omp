#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>
#include <algorithm>

int calculate(int n, int m){
    int prime_counter = 0;
    int limit = (int)sqrt(n);

    // Główna tablica wyników
    bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
    memset(result, true, (n - m + 1) * sizeof(bool));
        
    // Tablica na sito bazowe (do sqrt(n))
    bool* primeArray = (bool*)malloc((limit + 1) * sizeof(bool));
    memset(primeArray, true, (limit + 1) * sizeof(bool));
    primeArray[0] = false;
    primeArray[1] = false;
    
    // 1. POPRAWNE SITO BAZOWE (szukamy liczb pierwszych do sqrt(n))
	for (int i = 2; i*i*i*i <= n; i++) {
		if (primeArray[i] == true) {
			for (int j = i * i; j*j <= n; j+=i) {
				primeArray[j] = false;
			}
		}
	}

    // 2. SITO DOMENOWE (BLOKOWE) - LOKALNOŚĆ DOSTĘPU (wzorowane na k5.cpp)
    int blockSize = 65536; // Optymalna wartość 64 KB
    int numberOfBlocks = (n - m) / blockSize;
    if ((n - m) % blockSize != 0) { 
        numberOfBlocks++;
    }

    // Przetwarzanie sekwencyjne blok po bloku
    for (int i = 0; i < numberOfBlocks; i++) {
        int low = m + i * blockSize; 
        int high = low + blockSize - 1;
        if (high > n) {
            high = n;
        }

        // Wykreślanie wielokrotności tylko wewnątrz obecnego, małego bloku
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
                // Zapis lokalny do pamięci (gwarancja hitów w Cache L1/L2)
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
    int m = 0;
    int n = 1e8;
    double start_time, stop_time;

    // start_time = omp_get_wtime();
    int prime_counter = calculate(n, m);
    // stop_time = omp_get_wtime();

    printf("Liczba liczb pierwszych: %d\n", prime_counter);
    // printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);
    
    return 0;
}