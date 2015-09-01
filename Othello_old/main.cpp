#define DEF

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "Othello.h"
#include "Watch.h"
#include <cmath>
#include <thread>

template<typename T>
int r(int n, int value)
{
	T p1(value);
	T p2(value + 583);
	for(int i = 0; i < n; i++)
	{
		Othello::playDouble(&p1, &p2);
	}
}

void run(int n, int value, int size)
{
	switch (size)
	{
		case 0:
			r<CpuPlayer1>(n, value);
			break;
		case 1:
			r<TuplePlayer<1> >(n, value);
			break;
		case 2:
			r<TuplePlayer<2> >(n, value);
			break;
		case 3:
			r<TuplePlayer<3> >(n, value);
			break;
		case 4:
			r<TuplePlayer<4> >(n, value);
			break;
	}
}

void runTh(int n, int value, int size, int nThreads)
{
	std::thread **th = new std::thread *[nThreads];
	for(int i = 0; i < nThreads; i++)
		th[i] = new std::thread(run, n, value, size);
	for(int i = 0; i < nThreads; i++)
	{
		th[i]->join();
		delete th[i];
	}
	delete[] th;
}

int main(int argc, char **argv)
{
#define N_ITER 8
	int size = 0;
	int seed = 0;
	int nThreads = 8;
	if (argc > 1)
	{
		size = atoi(argv[1]);
	}
	if (argc > 2)
	{
		seed = atoi(argv[2]);
	}
	if (argc > 3)
	{
		nThreads = atoi(argv[3]);
	}
	srand(time(0));
	Vector<double, N_ITER-1> times;
	for(int t = 0; t < N_ITER; t++)
	{
		Watch<float> w;
		int nGames = 500;
		if (nThreads > 1)
			runTh(nGames, seed, size, nThreads);
		else
			run(nGames, seed, size);
		w.stop();
		if (t > 0)
			times.add(nThreads * nGames/w());
	}
	double avg(0);
	for(int i = 0; i < times.size(); i++)
		avg += times[i] / times.size();
	printf("GPS: %.1f\n", avg);

    return 0;
}
