
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define NDEBUG
#define nullptr 0

#define DEF __device__ __host__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "Header.h"
#include "Othello.h"
#include "Watch.h"
#include "WeightsOptimizer.h"
#include "CmaEsOptimizer.h"
#include "AnyLoader.h"

template<typename T>
cudaError cudaSetValue(UniversalLoader *dst, UniversalLoader *src, T *srcValuePos, T *value)
{
	char *dstPtr = reinterpret_cast<char *>(dst);
	char *srcPtr = reinterpret_cast<char *>(src);
	char *srcValPtr = reinterpret_cast<char *>(srcValuePos);

	char *dstValPtr = reinterpret_cast<char *>(dstPtr + (srcValPtr - srcPtr));

	return cudaMemcpy(dstValPtr, value, sizeof(T), cudaMemcpyHostToDevice);
}

template<typename T>
cudaError cudaCopyValue(UniversalLoader *dst, UniversalLoader *src, T *srcValuePos)
{
	return cudaSetValue(dst, src, srcValuePos, srcValuePos);
}

template<typename T>
cudaError cudaSetArray(UniversalLoader *dst, UniversalLoader *src, T **hostArray, int *hostSize, char *cudaData, char *hostData)
{
	int distance = reinterpret_cast<char *>(*hostArray) - hostData;
	T *pos = reinterpret_cast<T *>(cudaData + distance);
	if (*hostArray == nullptr)
		pos = nullptr;
	cudaError_t cudaStatus = cudaSetValue(dst, src, hostArray, &pos);
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaMemcpy failed!");
		return cudaStatus;
	}
	cudaStatus = cudaCopyValue(dst, src, hostSize);
	if (cudaStatus != cudaSuccess)
	{
        	fprintf(stderr, "cudaMemcpy failed!");
        	return cudaStatus;
	}

	return cudaStatus;
}

UniversalLoader *getCudaLoader(UniversalLoader *loader)
{
	UniversalLoader *result = nullptr;
	cudaError_t cudaStatus = cudaMalloc((void**)&result, sizeof(UniversalLoader));
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaMalloc failed!\n");
		goto Error;
	}
	cudaStatus = cudaMemset(result, 0, sizeof(UniversalLoader));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemset failed!");
        goto Error;
    }
	char *data;
	cudaStatus = cudaMalloc(&data, loader->dataSize * sizeof(*(loader->data)));
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaMalloc failed!\n");
		goto Error;
	}
	cudaStatus = cudaMemcpy(data, loader->data, loader->dataSize * sizeof(*(loader->data)), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaSetValue(result, loader, &loader->data, &data);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaCopyValue failed!");
        goto Error;
    }
	cudaStatus = cudaCopyValue(result, loader, &loader->dataSize);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaCopyValue(result, loader, &loader->type);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaCopyValue(result, loader, &loader->maxTuplePerPos);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaSetArray(result, loader, &loader->fields, &loader->nFields, data, loader->data);
	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaSetArray(result, loader, &loader->weights, &loader->nWeights, data, loader->data);
	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaSetArray(result, loader, &loader->tuples, &loader->nTuples, data, loader->data);
	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
	cudaStatus = cudaSetArray(result, loader, &loader->values, &loader->nValues, data, loader->data);
	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

	return result;
Error:

	return nullptr;
}

void removeCudaLoader(UniversalLoader *cudaLoader)
{
	UniversalLoader loader;
	UniversalLoader *normal = &loader;
	if (cudaLoader != nullptr)
	{
		int dst = reinterpret_cast<char *>(&normal->data) - reinterpret_cast<char *>(normal);
		char *pos = reinterpret_cast<char *>(cudaLoader) + dst;

		if (pos != nullptr)
			cudaFree(pos);
	
		cudaFree(cudaLoader);
	}
}

void getCudaLoader(AnyLoader *loader, char **data)
{
	cudaError_t cudaStatus = cudaMalloc((void**)data, loader->getRawDataSize());
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaMalloc failed!\n");
		goto Error;
	}
	
	cudaStatus = cudaMemcpy(*data, loader->getRawData(), loader->getRawDataSize(), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess)
	{
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

	return;
Error:
	data = nullptr;
	return;
}

void removeCudaData(char **data)
{
	if (*data != nullptr)
	{
		cudaFree(*data);
		*data = nullptr;
	}
}

__global__ void playGame(UniversalLoader *playerLoader, bool playerNeg, Board::EVALUATION_TYPE playerFreq, UniversalLoader **expertsLoaders, bool *expertsNeg, float expertsFreq, int nExperts, UniversalLoader *boards, Board::EVALUATION_TYPE **weights, Board::EVALUATION_TYPE *results, int gamesPerBlock, int seed)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;

	Rand r(seed + index);
	const int blockSize = blockDim.x;
	const int blockResultsOffset = blockIdx.x * gamesPerBlock;

	int nBoards = boards->getNValues();
	const int totalGames = nBoards * gamesPerBlock * nExperts;

	if (index == 0)
	{
		/*
		//NTuplePlayer<false, 6, 9, 3, 1> *p1 = new NTuplePlayer<false, 6, 9, 3, 1>(r.rand(), playerLoader);
		OthelloPlayer *p1 = OthelloPlayer::getPlayer(playerLoader, r.rand(), playerNeg);
		//printf("p1\n");
		NTuplePlayer<false, 6, 9, 3, 1> p2(r.rand(), expertsLoaders[0]);
		//printf("p2\n");
		p1->setRandomMoveFreq(playerFreq);
		p2.setRandomMoveFreq(expertsNeg[0]);
		//printf("f\n");

		Board b;
		//printf("Before\n");
		Othello::play(b, p1, &p2, r.rand());
		//printf("After\n");
		delete p1;*/
	}

	/*for(int i = threadIdx.x; i < gamesPerBlock; i += blockSize)
		results[i + blockResultsOffset] = 0;

	OthelloPlayer *player = OthelloPlayer::getPlayer(playerLoader, r.rand(), playerNeg);
	player->setRandomMoveFreq(playerFreq);
	__shared__ OthelloPlayer **experts;
	if (threadIdx.x == 0)
		experts = new OthelloPlayer*[nExperts];

	__syncthreads();
	for(int i = threadIdx.x; i < nExperts; i+=blockSize)
	{
		experts[i] = OthelloPlayer::getPlayer(expertsLoaders[i], r.rand(), expertsNeg[i]);
		experts[i]->setRandomMoveFreq(expertsFreq);
	}
	__syncthreads();

	Othello *othello = new Othello(player, experts, nExperts, r.rand());
	Board *tmpBoard = new Board();

	for(int i = threadIdx.x; i < totalGames; i += blockSize)
	{
		int board = i % nBoards;
		int tmp = i / nBoards;
		int game = tmp % gamesPerBlock;
		int expert = (tmp / gamesPerBlock) % nExperts;
		tmpBoard->copy(boards->getValues() + board * 64);
		Board::EVALUATION_TYPE result = othello->play(*tmpBoard, 0, expert + 1).item1;
		result = (1 - result) / (nBoards * nExperts);
		atomicAdd(results + blockResultsOffset + game, result);
	}

	delete tmpBoard;
	delete othello;

	__syncthreads();
	for(int i = threadIdx.x; i < nExperts; i+=blockSize)
		delete experts[i];
	__syncthreads();
	if (threadIdx.x == 0)
		delete[] experts;
	delete player;*/
}

class A
{
public:
	DEF A()
	{
		a = 0;
		printf("A()\t%d\n", a);
	}

	DEF A(const A &a)
	{
		this->a = a.a;
		printf("A(const A &a)\t%d\n", a);
	}

	int a;
};

DEF void f(char *data, unsigned dataSize)
{
	printf("Start f\n");

	PlayerLoader *loader = PlayerLoader::getLoader(data, dataSize, false);
	auto p1 = loader->getPlayer(0, false);
	auto p2 = loader->getPlayer(0, false);
	p1->setRandomMoveFreq(0);
	p2->setRandomMoveFreq(0);
	OthelloPlayer *players[] = { p1, p2 };
	//Othello oth(players, 2, 0);
	Othello *oth = new Othello(players, 2, 0);
	float sum = 0;
	int N = 1;

	for(int i = 0; i < N; i++)
		sum += oth->playDouble(0, 1).item1;
	
	printf("Result: %f\n", sum / N);

	delete oth;
	delete p2;
	delete p1;
	delete loader;
	printf("End f\n");
}

__global__ void playGame(char *data, unsigned dataSize)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;

	//printf("Start %d\n", index);

	//if (index == 0)
	{
		f(data, dataSize);
	}
}

/*class GPURunner : public GameRunner
{
protected:
	Rand rand;
	int nBlocks;
	int nThreads;
	int nGames;

	Configuration *conf;

	// Device player loader.
	UniversalLoader *cuda_pLoader;
	// Host array of device experts loaders;
	UniversalLoader **cuda_eLoaders;
	// Device array of device experts loaders.
	UniversalLoader **cudaArr_eLoaders;

	UniversalLoader *cuda_boards;
	
	bool pNeg;
	bool *eNeg;
	bool *cuda_eNeg;

	float pFreq;
	float eFreq;

	Board::EVALUATION_TYPE **cuda_weights;
	Board::EVALUATION_TYPE **cudaArr_weights;

	Board::EVALUATION_TYPE *cuda_result;

	void clear()
	{
		if (conf == nullptr)
			return;
		
		int nExperts = conf->getNExperts();
		if (cuda_pLoader != nullptr)
		{
			removeCudaLoader(cuda_pLoader);
			cuda_pLoader = nullptr;
		}
		if (cuda_eLoaders != nullptr)
		{
			for(int i = 0; i < nExperts; i++)
				removeCudaLoader(cuda_eLoaders[i]);
			delete[] cuda_eLoaders;
			cuda_eLoaders = nullptr;
		}
		if (cudaArr_eLoaders != nullptr)
		{
			cudaFree(cudaArr_eLoaders);
			cudaArr_eLoaders = nullptr;
		}
		if (eNeg != nullptr)
		{
			delete[] eNeg;
			eNeg = nullptr;
		}
		if (eNeg != nullptr)
		{
			cudaFree(cuda_eNeg);
			cuda_eNeg = nullptr;
		}
		if (cuda_boards != nullptr)
		{
			removeCudaLoader(cuda_boards);
			cuda_boards = nullptr;
		}
		if (cuda_result != nullptr)
		{
			cudaFree(cuda_result);
			cuda_result = nullptr;
		}
		if (cuda_weights != nullptr)
		{
			for(int i = 0; i < conf->getPlayer()->getNWeights(); i++)
			{
				cudaFree(cuda_weights[i]);
			}
			delete[] cuda_weights;
			cuda_weights = nullptr;
		}
		if (cudaArr_weights != nullptr)
		{
			cudaFree(cudaArr_weights);
			cudaArr_weights = nullptr;
		}
	}

	virtual void _setPlayerFreq(float freq)
	{
		pFreq = freq;
	}

	virtual void _setExpertsFreq(float freq)
	{
		eFreq = freq;
	}
public:
	GPURunner(int nBlocks, int nThreads, int gamesPerBlock, int seed) :
		rand(seed),
		nBlocks(nBlocks),
		nThreads(nThreads),
		nGames(gamesPerBlock),
		conf(nullptr),
		cuda_pLoader(nullptr),
		cudaArr_eLoaders(nullptr),
		cuda_eLoaders(nullptr),
		pNeg(false),
		eNeg(nullptr),
		cuda_eNeg(nullptr),
		pFreq(0.0f),
		eFreq(0.0f),
		cuda_boards(nullptr),
		cudaArr_weights(nullptr),
		cuda_weights(nullptr),
		cuda_result(nullptr)
	{
	}

	virtual ~GPURunner()
	{
		clear();
	}

	bool init(OptimizerConfiguration *conf)
	{
		clear();

		this->conf = conf;

		int nPlayers = nBlocks * nThreads;

		{
			cudaError_t result = cudaDeviceSetLimit(cudaLimitMallocHeapSize, nPlayers * 512 * 1024);
			if (result != cudaSuccess)
			{
				printf("Cannot allocate memory!\n");
				return false;
			}
		}

		int nExperts = conf->getNExperts();
		cuda_pLoader = getCudaLoader(conf->getPlayer());
		
		cuda_eLoaders = new UniversalLoader *[nExperts];
		for(int i = 0; i < nExperts; i++)
			cuda_eLoaders[i] = getCudaLoader(conf->getExperts()[i]);
		bool correct = cudaMalloc(&cudaArr_eLoaders, nExperts * sizeof(UniversalLoader *)) == cudaSuccess;
		correct &= cudaMemcpy(cudaArr_eLoaders, cuda_eLoaders, nExperts * sizeof(UniversalLoader *), cudaMemcpyHostToDevice) == cudaSuccess;

		pNeg = conf->getPlayerNeg();
		eNeg = new bool[nExperts];

		for(int i = 0; i < nExperts; i++)
			eNeg[i] = conf->getExpertsNeg()[i];

		correct &= cudaMalloc(&cuda_eNeg, nExperts * sizeof(bool)) == cudaSuccess;
		correct &= cudaMemcpy(cuda_eNeg, eNeg, nExperts * sizeof(bool), cudaMemcpyHostToDevice) == cudaSuccess;

		cuda_weights = new Board::EVALUATION_TYPE *[getPreferedNWeights()];
		int nWeights = conf->getPlayer()->getNWeights();
		for(int i = 0; i < getPreferedNWeights(); i++)
		{
			correct &= cudaMalloc(&cuda_weights[i], nWeights * sizeof(Board::EVALUATION_TYPE)) == cudaSuccess;
		}
		correct &= cudaMalloc(&cudaArr_weights, getPreferedNWeights() * sizeof(Board::EVALUATION_TYPE *)) == cudaSuccess;
		correct &= cudaMemcpy(cudaArr_weights, cuda_weights, getPreferedNWeights() * sizeof(Board::EVALUATION_TYPE *), cudaMemcpyHostToDevice) == cudaSuccess;

		correct &= cudaMalloc(&cuda_result, getPreferedNWeights() * sizeof(Board::EVALUATION_TYPE)) == cudaSuccess;

		cuda_boards = getCudaLoader(conf->getBoardsConf());

		if (!correct)
			clear();

		return correct;
	}

	bool run(Board::EVALUATION_TYPE *const*weights, int nWeights, Board::EVALUATION_TYPE *results)
	{
		if (nWeights != getPreferedNWeights() && nWeights != getNPlayers())
		{
			printf("Invalid number of weights (%d instead of %d)\n", nWeights, getPreferedNWeights());
			return false;
		}

		int wSize = conf->getPlayer()->getNWeights();
		bool correct = true;
		for(int i = 0; i < nWeights && correct; i++)
		{
			if (weights[i] != nullptr)
			{
				cudaError_t re = cudaMemcpy(cuda_weights[i], weights[i], wSize * sizeof(Board::EVALUATION_TYPE), cudaMemcpyHostToDevice);
				correct &= re == cudaSuccess;
			}
		}

		if (!correct)
		{
			printf("Copying weights failed %s\n", cudaGetErrorString(cudaGetLastError()));
			return false;
		}

		int nGames = nWeights / (nBlocks);
		if (nGames * nBlocks != nWeights)
		{
			printf("Wrong number of weights (%d instead of mul of %dx%d)\n", nWeights, nBlocks, nThreads);
			getchar();
			return false;
		}
		dim3 blocks(nBlocks);
		dim3 threads(nThreads);

		playGame<<<blocks, threads>>>(cuda_pLoader, pNeg, pFreq, cudaArr_eLoaders, cuda_eNeg, eFreq, conf->getNExperts(), cuda_boards, cudaArr_weights, cuda_result, nGames, rand.rand());

		correct = cudaGetLastError() == cudaSuccess;
		if (!correct)
		{
			fprintf(stderr, "playGame launch failed: %s\n", cudaGetErrorString(cudaGetLastError()));
			return false;
		}
    
		correct = cudaDeviceSynchronize() == cudaSuccess;
		if (!correct)
		{
			fprintf(stderr, "cudaDeviceSynchronize returned error code %d (%s) after launching addKernel!\n", cudaGetLastError(), cudaGetErrorString(cudaGetLastError()));
			getchar();
			return false;
		}

		correct = cudaMemcpy(results, cuda_result, nWeights * sizeof(Board::EVALUATION_TYPE), cudaMemcpyDeviceToHost) == cudaSuccess;
		if (!correct)
		{
			printf("Copying results failed\n");
			return false;
		}

		//printf("Correct so far\n");
		return true;
	}

	int getNPlayers()
	{
		return nBlocks;
	}

	int getPreferedNWeights()
	{
		return nBlocks * nGames;
	}

	int getMinimumNWeights()
	{
		return nBlocks;
	}
};*/

int main(int argc, char **argv)
{
	Watch<float> watch;
	{
		cudaError_t cudaStatus = cudaSetDevice(0);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?\n");
			return -1;
		}
		AnyLoader *loader = AnyLoader::getLoader("Valid\\SzubertJaskowskiKrawiec2013CTDL.oth");
		
		watch.start();
		f(loader->getRawData(), loader->getRawDataSize());
		watch.stop();
		printf("Done after %f\n", watch());
		/*getchar();
		return;*/

		char *data;
		getCudaLoader(loader, &data);
		
		dim3 blocks(1);
		dim3 threads(1);

		watch.start();
		playGame<<<blocks, threads>>>(data, loader->getRawDataSize());
		
		bool correct = cudaGetLastError() == cudaSuccess;
		if (!correct)
		{
			fprintf(stderr, "playGame launch failed: %s\n", cudaGetErrorString(cudaGetLastError()));
		}

		correct = cudaDeviceSynchronize() == cudaSuccess;
		if (!correct)
		{
			fprintf(stderr, "cudaDeviceSynchronize returned error code %d (%s) after launching addKernel!\n", cudaGetLastError(), cudaGetErrorString(cudaGetLastError()));
			getchar();
			return false;
		}

		watch.stop();
		printf("Done after %.2fs\n", watch());
		
		removeCudaData(&data);

		cudaStatus = cudaDeviceReset();
		if (cudaStatus != cudaSuccess)
		{
			fprintf(stderr, "cudaDeviceReset failed!\n");
		}

		getchar();

		return 0;
	}
}
