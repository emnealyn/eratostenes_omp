#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>

int main(){
	int m = 0;
	int n = 10e8;
	double start_time, stop_time;

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	memset(result, true, (n - m + 1) * sizeof(bool));

	bool* primeArray = (bool*)malloc((sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (sqrt(n) + 1) * sizeof(bool));

	start_time = omp_get_wtime();
	for (int i = 2; i*i*i*i <= n; i++) {
		if (primeArray[i] == true) {
			for (int j = i*i; j*j <= n; j+=i) {
				primeArray[j] = false;
			} 
		}
	}
	#pragma omp parallel for static //? Jaki podział pracy ?
	for (int i = 2; i*i <= n; i++){
		if (primeArray[i]) {
			int firstMultiple = (m / i);
			if (firstMultiple <= 1) {
				firstMultiple = i + i;
			} else if (m % i){ 
				firstMultiple = (firstMultiple * i) + i;
			} else {
				firstMultiple = (firstMultiple * i);
			}
			for (int j = firstMultiple; j <= n; j+=i) {
				result[j-m] = false;
			}
		}
	}
	stop_time = omp_get_wtime();

	printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);
}