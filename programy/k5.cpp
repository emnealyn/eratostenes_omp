#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>

int main(){
	int m = 0;
	int n = 1e8;
	double start_time, stop_time;

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	memset(result, true, (n - m + 1) * sizeof(bool));

	bool* primeArray = (bool*)malloc((int)(sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (int)(sqrt(n) + 1) * sizeof(bool));
	
	for (int i = 2; i*i*i*i <= n; i++) {
		if (primeArray[i] == true) {
			for (int j = i*i; j*j <= n; j+=i) {
				primeArray[j] = false;
			} 
		}
	}

	int blockSize = 65536; // Optymalna wartość 64 KB dla i5-13500H
	int numberOfBlocks = (n - m) / blockSize;
	if ((n - m) % blockSize != 0) { 
		numberOfBlocks++;
	}

	start_time = omp_get_wtime();
	#pragma omp parallel for schedule(static)
	for (int i = 0; i < numberOfBlocks; i++) {
		int low = m + i * blockSize; 
		int high = low + blockSize - 1;
		if (high > n) {
			high = n;
		}
		for (int j = 2; j * j<= high; j++) {
			if (primeArray[j]) {
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
	stop_time = omp_get_wtime();

	double duration = stop_time - start_time;
	double speed = (double)(n - m + 1) / duration;

	printf("Czas przetwarzania: %.6f s\n", duration);
	printf("Prędkość obliczeń: %.2E 1/s (liczba zbadanych liczb na sekundę obliczeń)\n", speed);

	free(result);
	free(primeArray);
	return 0;
}
