#ifndef GAME_RUNNER_H
#define GAME_RUNNER_H

#include "OptimizerConfiguration.h"

class GameRunner
{
	float playerFreq;
	float expertsFreq;
protected:
	Configuration *conf;
	virtual void _setPlayerFreq(float freq) = 0;
	virtual void _setExpertsFreq(float freq) = 0;
public:
	GameRunner() :
		playerFreq(0),
		expertsFreq(0)
	{

	}
	virtual ~GameRunner() { }

	virtual bool init(Configuration *conf) = 0;

	virtual bool run(Board::EVALUATION_TYPE *const*weights, int nWeights, Board::EVALUATION_TYPE *results) = 0;

	virtual Board::EVALUATION_TYPE runTest(Board::EVALUATION_TYPE *const weights)
	{
		int n = getMinimumNWeights();
		Board::EVALUATION_TYPE *results = new Board::EVALUATION_TYPE[n];
		Board::EVALUATION_TYPE **const w = new Board::EVALUATION_TYPE *[n];
		for (int i = 0; i < n; i++)
			w[i] = weights;
		if (!run(w, n, results))
			for(int i = 0; i < n; i++)
				results[i] = -1;
		Board::EVALUATION_TYPE result = 0;
		for (int i = 0; i < n; i++)
			result += 1 - results[i];
		delete[] w;
		delete[] results;
		return result / n;
	}

	virtual int getNPlayers() = 0;

	virtual int getPreferedNWeights() = 0;

	virtual int getMinimumNWeights()
	{
		return getPreferedNWeights();
	}

	void setExpertsFreq(float freq)
	{
		this->expertsFreq = freq;
		_setExpertsFreq(freq);
	}

	float getExpertsFreq()
	{
		return expertsFreq;
	}

	void setPlayerFreq(float freq)
	{
		this->playerFreq = freq;
		_setPlayerFreq(freq);
	}

	float getPlayerFreq()
	{
		return playerFreq;
	}
};

#endif // GAME_RUNNER_H