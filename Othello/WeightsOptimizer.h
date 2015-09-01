#ifndef WEIGHTS_OPTIMIZER_H
#define WEIGHTS_OPTIMIZER_H

#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include "Header.h"
#include "Board.h"
#include "Othello.h"
#include "OthelloPlayer.h"
#include "Random.h"
#include "IterationsStrategy.h"
#include "GameRunner.h"
#include "OptimizerConfiguration.h"

class WeightsOptimizer
{
protected:
	Random<float> rand;
public:
	WeightsOptimizer(int seed)
		: rand(seed)
	{
	}

	virtual ~WeightsOptimizer() { }

	virtual void optimize(Configuration *configuration, GameRunner *gameRunner) = 0;
protected:
	void saveWeights(const std::string &weightsFile, const std::string &bestWeightsFile, Board::EVALUATION_TYPE *weights, int nWeights, int totalIterations, int *iterations, int nLevels, Board::EVALUATION_TYPE result, bool isBest)
	{
		FILE *file = fopen(weightsFile.c_str(), "w");
		fprintf(file, "%d ", totalIterations);
		for(int i = 0; i < nLevels; i++)
			fprintf(file, "%d ", iterations[i]);
		fprintf(file, "%f\n", result);
		for (int i = 0; i < nWeights; i++)
			fprintf(file, "%f\n", weights[i]);
		fclose(file);

		if (isBest)
		{
			file = fopen(bestWeightsFile.c_str(), "w");
			fprintf(file, "%f\n", result);
			for (int i = 0; i < nWeights; i++)
				fprintf(file, "%f\n", weights[i]);
			fclose(file);
		}
	}

	void loadWeights(const std::string &weightsFile, const std::string &bestWeightsFile, Board::EVALUATION_TYPE **weights, Board::EVALUATION_TYPE *bestEvaluation, int nWeights, int *totalIterations, int *iterations, int nIterations)
	{
		Board::EVALUATION_TYPE *w = new Board::EVALUATION_TYPE[nWeights];
		*weights = w;
		FILE *file = fopen(weightsFile.c_str(), "r");
		if (fscanf(file, "%d ", totalIterations) != 1)
			totalIterations = 0;

		for(int i = 0; i < nIterations; i++)
			if (fscanf(file, "%d ", iterations + i) != 1)
				iterations[i] = 0;
		fclose(file);

		file = fopen(bestWeightsFile.c_str(), "r");
		if (fscanf(file, "%f\n", bestEvaluation) != 1)
			*bestEvaluation = 0;
		for (int i = 0; i < nWeights; i++)
		{
			float val;
			if (fscanf(file, "%f\n", &val) != 1)
			{
			  printf("Didn't read at %d\n", i);
			  val = 0;
			}
			w[i] = val;
		}
		fclose(file);
	}

	Board::EVALUATION_TYPE *getRandWeights(Configuration *configuration)
	{
		int nWeights = configuration->getPlayerLoader(0)->getNWeights();
		Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[nWeights];
		for (int i = 0; i < nWeights; i++)
			weights[i] = rand.getValue(-.1f, .1f);
		return weights;
	}

	Board::EVALUATION_TYPE *copyPlayerWeights(Configuration *configuration)
	{
        int nWeights = configuration->getPlayerLoader(0)->getNWeights();
		Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[nWeights];
		configuration->getPlayerLoader(0)->getWeights(weights);
        return weights;
	}
};

#endif // WEIGHTS_OPTIMIZER_H
