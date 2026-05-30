/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Matrix multiplication: C = A * B.
 * Host code.
 *
 * This sample implements matrix multiplication which makes use of shared memory
 * to ensure data reuse, the matrix multiplication is done using tiling approach.
 */

// System includes
#include <stdio.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>
#include <cuda_profiler_api.h>

// Helper functions and utilities to work with CUDA
#include <helper_functions.h>
#include <helper_cuda.h>

/**
 * Matrix multiplication (CUDA Kernel) on the device: C = A * B
 * wA is A's width and wB is B's width
 */
template <int BLOCK_SIZE> __global__ void MatrixMulCUDA(float *C, float *A,
    float *B, int wA,
    int wB) {
  // Block index
  int bx = blockIdx.x;
  int by = blockIdx.y;

  // Thread index
  int tx = threadIdx.x;
  int ty = threadIdx.y;

  // Index of the first sub-matrix of A processed by the block
  int aBegin = wA * BLOCK_SIZE * by;

  // Index of the last sub-matrix of A processed by the block
  int aEnd   = aBegin + wA - 1;

  // Step size used to iterate through the sub-matrices of A
  int aStep  = BLOCK_SIZE;

  // Index of the first sub-matrix of B processed by the block
  int bBegin = BLOCK_SIZE * bx;

  // Step size used to iterate through the sub-matrices of B
  int bStep  = BLOCK_SIZE * wB;

  // Csub is used to store the element of the block sub-matrix
  // that is computed by the thread
  float Csub = 0;

  // Loop over all the sub-matrices of A and B
  // required to compute the block sub-matrix
  for (int a = aBegin, b = bBegin;
       a <= aEnd;
       a += aStep, b += bStep) {
    // Declaration of the shared memory array As used to
    // store the sub-matrix of A
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];

    // Declaration of the shared memory array Bs used to
    // store the sub-matrix of B
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

    // Load the matrices from device memory
    // to shared memory; each thread loads
    // one element of each matrix
    As[ty][tx] = A[a + wA * ty + tx];
    Bs[ty][tx] = B[b + wB * ty + tx];

    // Synchronize to make sure the matrices are loaded
    __syncthreads();

    // Multiply the two matrices together;
    // each thread computes one element
    // of the block sub-matrix
#pragma unroll

    for (int k = 0; k < BLOCK_SIZE; ++k) {
      Csub += As[ty][k] * Bs[k][tx];
    }

    // Synchronize to make sure that the preceding
    // computation is done before loading two new
    // sub-matrices of A and B in the next iteration
    __syncthreads();
  }

  // Write the block sub-matrix to device memory;
  // each thread writes one element
  int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
  C[c + wB * ty + tx] = Csub;
}

// Step 2: 2 Results per thread (MatrixMulCUDA_2Res)
template <int BLOCK_SIZE> __global__ void MatrixMulCUDA_2Res(float *C, float *A,
    float *B, int wA,
    int wB) {
  int bx = blockIdx.x;
  int by = blockIdx.y;
  int tx = threadIdx.x;
  int ty = threadIdx.y;

  int aBegin = wA * BLOCK_SIZE * by;
  int aEnd   = aBegin + wA - 1;
  int aStep  = BLOCK_SIZE;

  int bBegin = 2 * BLOCK_SIZE * bx;
  int bStep  = BLOCK_SIZE * wB;

  float res1 = 0;
  float res2 = 0;

  for (int a = aBegin, b = bBegin;
       a <= aEnd;
       a += aStep, b += bStep) {
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float Bs[BLOCK_SIZE][2 * BLOCK_SIZE];

    As[ty][tx] = A[a + wA * ty + tx];
    Bs[ty][tx] = B[b + wB * ty + tx];
    Bs[ty][tx + BLOCK_SIZE] = B[b + wB * ty + tx + BLOCK_SIZE];

    __syncthreads();

#pragma unroll
    for (int k = 0; k < BLOCK_SIZE; ++k) {
        float aVal = As[ty][k];
        res1 += aVal * Bs[k][tx];
        res2 += aVal * Bs[k][tx + BLOCK_SIZE];
    }

    __syncthreads();
  }

  int c = wB * BLOCK_SIZE * by + 2 * BLOCK_SIZE * bx;
  C[c + wB * ty + tx] = res1;
  C[c + wB * ty + tx + BLOCK_SIZE] = res2;
}

// Step 3: 4 Results per thread (MatrixMulCUDA_4Res)
template <int BLOCK_SIZE> __global__ void MatrixMulCUDA_4Res(float *C, float *A,
    float *B, int wA,
    int wB) {
  int bx = blockIdx.x;
  int by = blockIdx.y;
  int tx = threadIdx.x;
  int ty = threadIdx.y;

  int aBegin = wA * 2 * BLOCK_SIZE * by;
  int aEnd   = aBegin + wA - 1;
  int aStep  = BLOCK_SIZE;

  int bBegin = 2 * BLOCK_SIZE * bx;
  int bStep  = BLOCK_SIZE * wB;

  float res00 = 0, res01 = 0, res10 = 0, res11 = 0;

  for (int a = aBegin, b = bBegin;
       a <= aEnd;
       a += aStep, b += bStep) {
    __shared__ float As[2 * BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float Bs[BLOCK_SIZE][2 * BLOCK_SIZE];

    As[ty][tx] = A[a + wA * ty + tx];
    As[ty + BLOCK_SIZE][tx] = A[a + wA * (ty + BLOCK_SIZE) + tx];
    Bs[ty][tx] = B[b + wB * ty + tx];
    Bs[ty][tx + BLOCK_SIZE] = B[b + wB * ty + tx + BLOCK_SIZE];

    __syncthreads();

#pragma unroll
    for (int k = 0; k < BLOCK_SIZE; ++k) {
        float a0 = As[ty][k];
        float a1 = As[ty + BLOCK_SIZE][k];
        float b0 = Bs[k][tx];
        float b1 = Bs[k][tx + BLOCK_SIZE];
        res00 += a0 * b0;
        res01 += a0 * b1;
        res10 += a1 * b0;
        res11 += a1 * b1;
    }

    __syncthreads();
  }

  int c = wB * 2 * BLOCK_SIZE * by + 2 * BLOCK_SIZE * bx;
  C[c + wB * ty + tx] = res00;
  C[c + wB * ty + tx + BLOCK_SIZE] = res01;
  C[c + wB * (ty + BLOCK_SIZE) + tx] = res10;
  C[c + wB * (ty + BLOCK_SIZE) + tx + BLOCK_SIZE] = res11;
}

// Step 1: New initialization logic
void MatrixInitA(float *data, int h, int w) {
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            data[i * w + j] = (i % 100) * 0.01f;
        }
    }
}

void MatrixInitB(float *data, int h, int w) {
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            data[i * w + j] = (j % 100) * 0.01f;
        }
    }
}

// Step 1: Host reference function
void MatrixMulHost(float *C, float *A, float *B, int hA, int wA, int wB) {
    for (int i = 0; i < hA; ++i) {
        for (int j = 0; j < wB; ++j) {
            double sum = 0;
            for (int k = 0; k < wA; ++k) {
                sum += (double)A[i * wA + k] * (double)B[k * wB + j];
            }
            C[i * wB + j] = (float)sum;
        }
    }
}

enum KernelType {
    ORIGINAL,
    TWO_RES,
    FOUR_RES
};

void RunBenchmark(KernelType type, int block_size, int dimsAx, int dimsAy, int dimsBx, int dimsBy,
                  float *d_A, float *d_B, float *d_C, float *h_C, float *h_C_ref,
                  cudaStream_t stream, cudaEvent_t start, cudaEvent_t stop) {
    
    const char* kernel_name = "";
    dim3 threads, grid;

    if (type == ORIGINAL) {
        kernel_name = "Original";
        threads = dim3(block_size, block_size);
        grid = dim3(dimsBx / threads.x, dimsAy / threads.y);
    } else if (type == TWO_RES) {
        kernel_name = "2-Results/thread";
        threads = dim3(block_size, block_size);
        grid = dim3(dimsBx / (2 * threads.x), dimsAy / threads.y);
    } else if (type == FOUR_RES) {
        kernel_name = "4-Results/thread";
        threads = dim3(block_size, block_size);
        grid = dim3(dimsBx / (2 * threads.x), dimsAy / (2 * threads.y));
    }

    printf("Running %s (BLOCK_SIZE=%d)... ", kernel_name, block_size);

    // Warmup
    if (type == ORIGINAL) {
        if (block_size == 16) MatrixMulCUDA<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        else MatrixMulCUDA<32><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
    } else if (type == TWO_RES) {
        if (block_size == 16) MatrixMulCUDA_2Res<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        else MatrixMulCUDA_2Res<32><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
    } else if (type == FOUR_RES) {
        if (block_size == 16) MatrixMulCUDA_4Res<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        // BLOCK_SIZE=32 blocked as per instructions
    }
    checkCudaErrors(cudaStreamSynchronize(stream));

    checkCudaErrors(cudaEventRecord(start, stream));
    int nIter = 10;
    for (int j = 0; j < nIter; j++) {
        if (type == ORIGINAL) {
            if (block_size == 16) MatrixMulCUDA<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
            else MatrixMulCUDA<32><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        } else if (type == TWO_RES) {
            if (block_size == 16) MatrixMulCUDA_2Res<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
            else MatrixMulCUDA_2Res<32><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        } else if (type == FOUR_RES) {
            if (block_size == 16) MatrixMulCUDA_4Res<16><<<grid, threads, 0, stream>>>(d_C, d_A, d_B, dimsAx, dimsBx);
        }
    }
    checkCudaErrors(cudaEventRecord(stop, stream));
    checkCudaErrors(cudaEventSynchronize(stop));

    float msecTotal = 0.0f;
    checkCudaErrors(cudaEventElapsedTime(&msecTotal, start, stop));
    float msecPerIter = msecTotal / nIter;
    double flops = 2.0 * (double)dimsAx * (double)dimsAy * (double)dimsBx;
    double gigaFlops = (flops * 1.0e-9) / (msecPerIter / 1000.0);

    checkCudaErrors(cudaMemcpyAsync(h_C, d_C, dimsBx * dimsAy * sizeof(float), cudaMemcpyDeviceToHost, stream));
    checkCudaErrors(cudaStreamSynchronize(stream));

    bool correct = true;
    for (int i = 0; i < dimsBx * dimsAy; i++) {
        double cpu_ref = (double)h_C_ref[i];
        double gpu_res = (double)h_C[i];
        double rel_err = fabs(gpu_res - cpu_ref) / (fabs(cpu_ref) > 1.0 ? fabs(cpu_ref) : 1.0);
        if (rel_err > 1e-2) {
            printf("Error at %d: GPU=%f, CPU=%f, rel_err=%E\n", i, gpu_res, cpu_ref, rel_err);
            correct = false;
            break;
        }
    }

    printf("Done. Performance= %.2f GFlop/s, Time= %.3f msec, Result: %s\n",
           gigaFlops, msecPerIter, correct ? "PASS" : "FAIL");
}

int main(int argc, char **argv) {
    printf("[Matrix Multiply Using CUDA] - Starting...\n");

    int dev = findCudaDevice(argc, (const char **)argv);

    // Step 4: Fixed size 3200x3200
    int size = 3200;
    dim3 dimsA(size, size, 1);
    dim3 dimsB(size, size, 1);
    
    unsigned int mem_size_A = sizeof(float) * dimsA.x * dimsA.y;
    unsigned int mem_size_B = sizeof(float) * dimsB.x * dimsB.y;
    unsigned int mem_size_C = sizeof(float) * dimsB.x * dimsA.y;

    float *h_A, *h_B, *h_C, *h_C_ref;
    checkCudaErrors(cudaMallocHost(&h_A, mem_size_A));
    checkCudaErrors(cudaMallocHost(&h_B, mem_size_B));
    checkCudaErrors(cudaMallocHost(&h_C, mem_size_C));
    checkCudaErrors(cudaMallocHost(&h_C_ref, mem_size_C));

    MatrixInitA(h_A, dimsA.y, dimsA.x);
    MatrixInitB(h_B, dimsB.y, dimsB.x);

    printf("Computing CPU reference (may take a while)... ");
    fflush(stdout);
    MatrixMulHost(h_C_ref, h_A, h_B, dimsA.y, dimsA.x, dimsB.x);
    printf("Done.\n");

    float *d_A, *d_B, *d_C;
    checkCudaErrors(cudaMalloc(&d_A, mem_size_A));
    checkCudaErrors(cudaMalloc(&d_B, mem_size_B));
    checkCudaErrors(cudaMalloc(&d_C, mem_size_C));

    cudaStream_t stream;
    checkCudaErrors(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    checkCudaErrors(cudaMemcpyAsync(d_A, h_A, mem_size_A, cudaMemcpyHostToDevice, stream));
    checkCudaErrors(cudaMemcpyAsync(d_B, h_B, mem_size_B, cudaMemcpyHostToDevice, stream));

    cudaEvent_t start, stop;
    checkCudaErrors(cudaEventCreate(&start));
    checkCudaErrors(cudaEventCreate(&stop));

    printf("\nSummary of results (Matrix Size: %dx%d):\n", size, size);
    printf("--------------------------------------------------------------------------------\n");

    RunBenchmark(ORIGINAL, 16, dimsA.x, dimsA.y, dimsB.x, dimsB.y, d_A, d_B, d_C, h_C, h_C_ref, stream, start, stop);
    RunBenchmark(ORIGINAL, 32, dimsA.x, dimsA.y, dimsB.x, dimsB.y, d_A, d_B, d_C, h_C, h_C_ref, stream, start, stop);
    RunBenchmark(TWO_RES, 16, dimsA.x, dimsA.y, dimsB.x, dimsB.y, d_A, d_B, d_C, h_C, h_C_ref, stream, start, stop);
    RunBenchmark(TWO_RES, 32, dimsA.x, dimsA.y, dimsB.x, dimsB.y, d_A, d_B, d_C, h_C, h_C_ref, stream, start, stop);
    RunBenchmark(FOUR_RES, 16, dimsA.x, dimsA.y, dimsB.x, dimsB.y, d_A, d_B, d_C, h_C, h_C_ref, stream, start, stop);

    printf("--------------------------------------------------------------------------------\n");

    checkCudaErrors(cudaFreeHost(h_A));
    checkCudaErrors(cudaFreeHost(h_B));
    checkCudaErrors(cudaFreeHost(h_C));
    checkCudaErrors(cudaFreeHost(h_C_ref));
    checkCudaErrors(cudaFree(d_A));
    checkCudaErrors(cudaFree(d_B));
    checkCudaErrors(cudaFree(d_C));
    checkCudaErrors(cudaEventDestroy(start));
    checkCudaErrors(cudaEventDestroy(stop));
    checkCudaErrors(cudaStreamDestroy(stream));

    return 0;
}
