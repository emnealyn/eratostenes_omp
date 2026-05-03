#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>

int main(){
	int m = 2;
	int n = 1e9;
	double start_time, stop_time;

	int prime_counter = 0;

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));

	int limit = sqrt(n);
	bool* primeArray = (bool*)malloc((limit + 1) * sizeof(bool));
	memset(primeArray, true, (limit + 1) * sizeof(bool));

	for (int i = 2; i * i <= limit; i++) {
		if (primeArray[i]) {
			for (int j = i * i; j <= limit; j += i) {
				primeArray[j] = false;
			} 
		}
	}

	int blockSize = 65536;
	int numberOfBlocks = (n - m) / blockSize + ((n - m) % blockSize != 0);

	#pragma omp parallel for schedule(guided)
	for (int i = 0; i < numberOfBlocks; i++) {
		int low = m + i * blockSize; 
		int high = low + blockSize - 1;
		if (high > n) high = n;

		memset(&result[low - m], true, (high - low + 1) * sizeof(bool));

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

	if (m <= 0 && n >= 0) result[0 - m] = false;
	if (m <= 1 && n >= 1) result[1 - m] = false;

	#pragma omp parallel for reduction(+:prime_counter)
	for (int i = m; i <= n; i++) {
		if (result[i - m]) {
			prime_counter++;
		}
	}
	
	printf("Liczba liczb pierwszych: %d\n", prime_counter);

	free(result);
	free(primeArray);
	return 0;
}
