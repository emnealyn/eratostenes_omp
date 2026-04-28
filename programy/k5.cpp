#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>


int main(){
	int m = 0;
	int n = 1e8;
	double start_time, stop_time;

	// start_time = omp_get_wtime();
	int prime_counter=0;
	int max_threads = omp_get_max_threads();

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	memset(result, true, (n - m + 1) * sizeof(bool));

	bool* primeArray = (bool*)malloc((sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (sqrt(n) + 1) * sizeof(bool));

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
	#pragma omp parallel
	{
	int thread_id = omp_get_thread_num();
	if(thread_id == 0){
		int num_threads = omp_get_num_threads();
		printf("liczba wotkow:  %d\n", num_threads);
	}
	
	#pragma omp parallel for schedule(guided)
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
	}

	if (m <= 0 && n >= 0) result[0 - m] = false;
    if (m <= 1 && n >= 1) result[1 - m] = false;

    for (int i = m; i <= n; i++) {
        if (result[i - m] == true) {
            prime_counter++;
        }
    }

	free(result);
    free(primeArray);
	// stop_time = omp_get_wtime();

	// double duration = stop_time - start_time;
	// double speed = (double)(n - m + 1) / duration;

	// printf("Czas przetwarzania: %.6f s\n", duration);
	// printf("Prędkość obliczeń: %.2E 1/s (liczba zbadanych liczb na sekundę obliczeń)\n", speed);

	printf("Liczba liczb pierwszych: %d\n", prime_counter);
	// printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);
}