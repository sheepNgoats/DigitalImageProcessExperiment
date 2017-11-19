#ifndef  __MEDIANFILTER_CU_
#define  __MEDIANFILTER_CU_

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iostream>
#define datasize 100

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}
__device__ int wb_checkColorSpace(double x) {
	if (x > 255)
		return 255;
	if (x < 0)
		return 0;
	return x;
}

__global__ void white_balance(int *In, int *Out, int Width, int Height, double color_sum, double RGB_sum)
{
	int y = blockDim.y * blockIdx.y + threadIdx.y;
	int x = blockDim.x * blockIdx.x + threadIdx.x;
	//需要调整的RGB分量的增益
	double K = (RGB_sum) / (3 * color_sum);
	if (x <= Width && x >= 0 && y <= Height && y >= 0)
	{
		Out[y* Width + x] = wb_checkColorSpace((*(In + Width * y + x))*K);
	}
}

extern "C" void white_balance_host(int *pixel, int Width, int Height, double color_sum, double RGB_sum)
{
	int *pixelIn, *pixelOut;
	dim3 dimBlock(32, 32);
	dim3 dimGrid((Width + dimBlock.x - 1) / dimBlock.x, (Height + dimBlock.y -
		1) / dimBlock.y);
	checkCudaErrors(cudaMalloc((void**)&pixelIn, sizeof(int) * Width * Height));
	checkCudaErrors(cudaMalloc((void**)&pixelOut, sizeof(int) * Width * Height));

	checkCudaErrors(cudaMemcpy(pixelIn, pixel, sizeof(int) * Width * Height, cudaMemcpyHostToDevice));

	white_balance << <dimGrid, dimBlock >> > (pixelIn, pixelOut, Width , Height, color_sum, RGB_sum);

	checkCudaErrors(cudaMemcpy(pixel, pixelOut, sizeof(int) * Width * Height, cudaMemcpyDeviceToHost));


	cudaFree(pixelIn);
	cudaFree(pixelOut);
}

#endif // ! __MEDIANFILTER_KERNEL_CU_