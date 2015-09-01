#ifndef ITERATIONS_STRATEGY_H
#define ITERATIONS_STRATEGY_H

#include <cmath>

class IterationsStrategy
{
public:
	virtual ~IterationsStrategy() { }

	/// Liczba epok, w których zmieniaj¹ siê prawdopodobieñstwa losowego ruchu.
	virtual int getNEpoch() = 0;
	/// Informuje, czy dana epoka ma zostaæ przeprowadzona.
	virtual bool runEpoch(int epoch) = 0;
	/// Zwraca prawdopodobieñstwo ruchu losowego ekspertów.
	virtual float getEpochFreq(int epoch) = 0;
	/// Zwraca odsetek wygranych gier, który najlepszy gracz przekroczyæ, ¿eby zakoñczyæ epokê.
	virtual float epochBorder(int epoch) = 0;
	/// Zwraca liczbê iteracji w danej epoce.
	virtual int getNIteration(int epoch) = 0;
	/// Zwraca liczbê gier do rozegrania w konkretnej epoce i iteracji.
	virtual int getNGames(int epoch, int iteration) = 0;
};

class StandardStrategy : public IterationsStrategy
{
	int n;
	int bigN;
public:
	StandardStrategy(int n, int bigN) :
		n(n),
		bigN(bigN)
	{
		if (this->bigN < n)
			this->bigN = n;
	}

	int getNEpoch()
	{
		return bigN;
	}
	bool runEpoch(int epoch)
	{
		return true;
	}
	float getEpochFreq(int epoch)
	{
		if (epoch > n)
			epoch = n;

		return 1 - powf(epoch / (float)n, .35f);
	}
	float epochBorder(int epoch)
	{
		if (epoch > n)
			epoch = n;

		return 0.9f + 0.1f * powf(epoch / (float)n, 8) + .09f * powf((n - epoch) / (float)n, .9f);
		return 0.95f;
		if (epoch < 5)
			return 0.95f;
		else if (epoch < 18)
			return 0.9f;
		else if (epoch < 20)
			return 0.95f;
		else
			return 1.0f;
	}
	int getNIteration(int epoch)
	{
	    if (epoch == bigN)
            return 200;

		if (epoch >= n)
			return 100;
		else
			return 100;
	}
	int getNGames(int epoch, int iteration)
	{
		return 10;
	}
};

#endif // ITERATIONS_STRATEGY_H