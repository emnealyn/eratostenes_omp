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
	
	int blockSize = 2;
	int numberOfBlocks = (n - m) / blockSize;
	if ((n - m) % blockSize != 0) { 
		numberOfBlocks++;
	}
	#pragma omp parallel for static // ?
	for (int i = 0; i < numberOfBlocks; i++) {
		int low = m + i * blockSize; int high = m + i * blockSize + blockSize;
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