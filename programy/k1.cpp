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

	bool* primeArray = (bool*)malloc((sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (sqrt(n) + 1) * sizeof(bool));

	start_time = omp_get_wtime();

	for (int i = 2; i * i <= n; i++) {
		for (int j = 2; j * j <= i; j++) {
			if (primeArray[j] == true && i % j == 0) { primeArray[i] = false; 
				break;
			}
		}
	}
	for (int i = m; i <= n; i++){
		for (int j = 2; j * j <= i; j++){
			if (primeArray[j] == true && i % j == 0) {
				result[i - m] = false;
				break; 
			}
		}
	}
	stop_time = omp_get_wtime();

	printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);


}