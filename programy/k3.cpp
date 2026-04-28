#include <cstdlib> 
#include <cstring> 
#include <cmath>  
#include <cstdio>
#include <omp.h>

int calculate(int n, int m){
	int prime_counter=0;
	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	memset(result, true, (n - m + 1) * sizeof(bool));
		
	bool* primeArray = (bool*)malloc((sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (sqrt(n) + 1) * sizeof(bool));
	
	for (int i = 2; i*i*i*i <= n; i++) {
		if (primeArray[i] == true) {
			for (int j = i * i; j*j <= n; j+=i) {
				primeArray[j] = false;
			}
		}
	}
	for (int i = 2; i*i <= n; i++){
		if (primeArray[i]) {
			int firstMultiple = (m / i);
			if (firstMultiple <= 1) {
				firstMultiple = i + i;
			} else if (m % i) { 
				firstMultiple = (firstMultiple * i) + i;
			} else {
				firstMultiple = (firstMultiple * i);
			}
			for (int j = firstMultiple; j <= n; j+=i) { 
				result[j-m] = false;
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

	start_time = omp_get_wtime();
	int prime_counter = 0;
	prime_counter = calculate(n, m);
	stop_time = omp_get_wtime();

	printf("Liczba liczb pierwszych: %d\n", prime_counter);
	printf("Czas trwania obliczen - wallclock %f sekund \n", stop_time-start_time);
}