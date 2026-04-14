#include <cstdlib> 
#include <cstring> 
#include <cmath>  

int main(){
	int m = 0;
	int n = 10;

	bool* result = (bool*)malloc((n - m + 1) * sizeof(bool));
	memset(result, true, (n - m + 1) * sizeof(bool));

	bool* primeArray = (bool*)malloc((sqrt(n) + 1) * sizeof(bool));
	memset(primeArray, true, (sqrt(n) + 1) * sizeof(bool));

	for (int i = 2; i * i <= n; i++) {
		for (int j = 2; j * j <= i; j++) {
			if (primeArray[j] == true && i % j == 0) { 
				primeArray[i] = false; break; 
			}
		}
	}
	#pragma omp parallel
	{
	#pragma omp for static //? Jaki podział pracy 
	for (int i = m; i <= n; i++){
		for (int j = 2; j * j <= i; j++){
			if (primeArray[j] == true && i % j == 0) { 
				result[i - m] = false;
				break;
				}
			}
		}
	} 

}