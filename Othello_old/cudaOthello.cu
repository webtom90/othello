
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define NDEBUG

#define DEF __device__ __host__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "Othello.h"
#include "Watch.h"

cudaError_t runGame(Watch<float> &w, int *c, float *res1, float *res2, const unsigned height, const unsigned width, int time, int size);

template<typename T>
__device__ void r(int *c, float *res1, float *res2, int n)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int value = c[index];
	T p1(value);
	T p2(value + 583);
	int counter = 0;
	float r1 = 0;
	float r2 = 0;
	for(int i = 0; i < n; i++)
	{
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result = Othello::playDouble(&p1, &p2);
		r1 += result.item1;
		r2 += result.item2;
		counter++;
	}
	c[index] = counter;
	res1[index] = r1;
	res2[index] = r2;
}

__device__ void run(int *c, float *res1, float *res2, int n, int size)
{
	switch (size)
	{
		case 0:
			r<CpuPlayer1>(c, res1, res2, n);
			break;
		case 1:
			r<TuplePlayer<1> >(c, res1, res2, n);
			break;
		case 2:
			r<TuplePlayer<2> >(c, res1, res2, n);
			break;
		case 3:
			r<TuplePlayer<3> >(c, res1, res2, n);
			break;
		case 4:
			r<TuplePlayer<4> >(c, res1, res2, n);
			break;
	}
}

__global__ void runOnGPU(int *array, float *res1, float *res2, int time, int size)
{
	run(array, res1, res2, time, size);
}

int main(int argc, char **argv)
{
#define N_ITER 8
	int count = 1;
	int seed = 0;
	int size = 1;
	if (argc > 1)
	{
		seed = atoi(argv[1]);
	}
	if (argc > 2)
	{
		size = atoi(argv[2]);
	}
	srand(0);
	Vector<double, N_ITER-1> times;
	for(int t = 0; t < N_ITER; t++)
	{
	Watch<float> w;
	const int width = 64;
	const int height = 21 * 1;
	const int arraySize = width * height;
	int c[arraySize];
	for(int i = 0; i < arraySize; i++)
		c[i] = seed;
	float r1[arraySize] = { 0 };
	float r2[arraySize] = { 0 };

	// Add vectors in parallel.
	cudaError_t cudaStatus = runGame(w, c, r1, r2, height, width, count, size);
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "addWithCuda failed!");
		getchar();
		return 1;
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaDeviceReset failed!");
		getchar();
		return 1;
	}
	
	int totalCounter = 0;
	float w1 = 0;
	float w2 = 0;
	for(int i = 0; i < arraySize; i++)
	{
		if (c[i] <= 0)
		{
			printf("Zly wynik: %d (%d)\n", i, c[i]);
			break;
		}
		totalCounter += c[i];
		w1 += r1[i];
		w2 += r2[i];
	}

	if (t > 0)
		times.add(totalCounter/w());
	}
	double avg(0), dev(0);
	for(int i = 0; i < times.size(); i++)
		avg += times[i] / times.size();
	for(int i = 0; i < times.size(); i++)
		dev += (times[i] - avg) * (times[i] - avg);
	dev = sqrt(dev / times.size());
	printf("GPS: %.1f, %.3f\n", avg, dev);

    return 0;
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t runGame(Watch<float> &w, int *c, float *res1, float *res2, const unsigned height, const unsigned width, int time, int N)
{
    const int size = height * width;
    int *dev_c = 0;
    float *dev_r1 = 0;
    float *dev_r2 = 0;
    dim3 blockGridRows(height);
    dim3 threadBlockRows(width);
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

	// Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_r1, size * sizeof(float));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_r2, size * sizeof(float));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_c, c, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
    
    cudaStatus = cudaFuncSetCacheConfig(runOnGPU, cudaFuncCachePreferL1);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSetCacheConfig launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }

    w.start();
    // Launch a kernel on the GPU with one thread for each element.
    runOnGPU<<<blockGridRows, threadBlockRows>>>(dev_c, dev_r1, dev_r2, time, N);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "runOnGPU launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching runOnGPU!\n", cudaStatus);
        goto Error;
    }
    w.stop();

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(res1, dev_r1, size * sizeof(float), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(res2, dev_r2, size * sizeof(float), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_r1);
    cudaFree(dev_r2);
    
    return cudaStatus;
}
