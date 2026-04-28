#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>

int main(){
	int m = 0;
	int n = 10e8; // Zwiększone n dla lepszych pomiarów cache

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	bool* primeArray = (bool*)malloc((int)(sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (int)(sqrt(n) + 1) * sizeof(bool));
	
	// Precompute small primes (Sieve of Eratosthenes up to sqrt(n))
	for (int i = 2; i * i <= sqrt(n); i++) {
		if (primeArray[i]) {
			for (int j = i * i; j <= sqrt(n); j += i) {
				primeArray[j] = false;
			}
		}
	}

	printf("%-15s | %-15s\n", "BlockSize", "Czas [s]");
	printf("----------------------------------------\n");

	// Testowanie różnych wielkości bloków (potęgi 2)
	for (int blockSize = 1024; blockSize <= (n - m); blockSize *= 2) {
		memset(result, true, (n - m + 1) * sizeof(bool));
		int numberOfBlocks = (n - m) / blockSize;
		if ((n - m) % blockSize != 0) numberOfBlocks++;

		double start_time = omp_get_wtime();
		#pragma omp parallel for schedule(static)
		for (int i = 0; i < numberOfBlocks; i++) {
			int low = m + i * blockSize; 
			int high = low + blockSize - 1;
			if (high > n) high = n;

			for (int j = 2; j * j <= high; j++) {
				if (primeArray[j]) {
					int firstMultiple = (low / j) * j;
					if (firstMultiple < low) firstMultiple += j;
					if (firstMultiple == j) firstMultiple += j;

					for (int k = firstMultiple; k <= high; k += j) {
						result[k - m] = false;
					}
				}
			}
		}
		double stop_time = omp_get_wtime();
		printf("%-15d | %-15f\n", blockSize, stop_time - start_time);
	}

	free(result);
	free(primeArray);
	return 0;
}
