#ifndef CMA_ES_OPTIMIZER
#define CMA_ES_OPTIMIZER

#include "cma-es/cmaes.h"
#include "WeightsOptimizer.h"
#include "Logger.h"
#include "Watch.h"

class AbsCmaEsOptimizer : public WeightsOptimizer
{
	int run(int *iterationsCounter, int level, int nLevels)
	{
		if (level == createCmaesLevel() - 1)
			createCmaes(bestWeights, currentConfiguration->getPlayerLoader(0)->getNWeights());

		int retLevel = level;
		if (level == nLevels)
		{
			learn();
			iterationsCounter[-1]++;
			if (getCmaes()->testForTermination())
            {
                retLevel = createCmaesLevel();
                printf("Flat\n");
            }
		}
		else
		{
			onBeginLevel(iterationsCounter, level, nLevels);
			iterationsCounter[level] = 0;
			int nIt = getNIterations(iterationsCounter, level, nLevels);
			for(int it = 0; it < nIt; it++)
			{
				iterationsCounter[level] = it;
				beforeIteration(iterationsCounter, level, nLevels);
				retLevel = run(iterationsCounter, level + 1, nLevels);
				afterIteration(iterationsCounter, level, nLevels);
				if (retLevel < level)
					break;
			}
			retLevel = std::min(onEndLevel(iterationsCounter, level, nLevels), retLevel);
		}

		return retLevel;
	}

	bool learn()
	{
		onBeginLearning();

		if (createCmaesLevel() == 0)
			createCmaes(bestWeights, currentConfiguration->getPlayerLoader(0)->getNWeights());

		Board::EVALUATION_TYPE *const*pop = cmaes->samplePopulation();
		if (!currentGameRunner->run(pop, N, arFunvals))
		{
			printf("Error while evaluating weights\n");
			return false;
		}
		/*for(int i = 0; i < N; i++)
		{
			arFunvals[i] = this->rand.getValue(1);
		}*/
		cmaes->updateDistribution(arFunvals);

		onEndLearning();

		return true;
	}

	void createCmaes(const Board::EVALUATION_TYPE *weights, int nWeights)
	{
		if (cmaes != nullptr)
			delete cmaes;

		cmaes = new CMAES<Board::EVALUATION_TYPE>(rand.rand());
		Board::EVALUATION_TYPE *stddev = new Board::EVALUATION_TYPE[nWeights];
		for (int i = 0; i < nWeights; i++)
			stddev[i] = 1;
		Parameters<Board::EVALUATION_TYPE> parameters;
		parameters.lambda = N;
		parameters.init(nWeights, weights, stddev);
		delete[] stddev;
		arFunvals = cmaes->init(parameters);
	}
protected:
	CMAES<Board::EVALUATION_TYPE> *getCmaes()
	{
		return cmaes;
	}

	CMAES<Board::EVALUATION_TYPE> *cmaes;
	Board::EVALUATION_TYPE *arFunvals;
	int N;
	Board::EVALUATION_TYPE *bestWeights;
	Board::EVALUATION_TYPE bestWeightsEvaluation;

	Configuration *currentConfiguration;
	GameRunner *currentGameRunner;

	std::string weightsFile;
	std::string bestWeightsFile;

	virtual bool continueLearning()
	{
	    return false;
	}
public:
	AbsCmaEsOptimizer(int seed)
		: WeightsOptimizer(seed),
		cmaes(nullptr)
	{
	}

	~AbsCmaEsOptimizer()
	{
		if (cmaes != nullptr)
		{
			delete cmaes;
			cmaes = nullptr;
		}
	}

	void setFiles(std::string weightsFile, std::string bestWeightsFile)
	{
		this->weightsFile = weightsFile;
		this->bestWeightsFile = bestWeightsFile;
	}

	void optimize(Configuration *configuration, GameRunner *gameRunner)
	{
		currentConfiguration = configuration;
		currentGameRunner = gameRunner;

		if (!gameRunner->init(configuration))
		{
			printf("Error occured while initializing runner\n");
			return;
		}

		N = gameRunner->getPreferedNWeights();
		if (continueLearning())
            bestWeights = copyPlayerWeights(configuration);
        else
            bestWeights = getRandWeights(configuration);
		bestWeightsEvaluation = 0;

		onBeginOptimizing(configuration, gameRunner);

		int nLevels = getNLevels();
		int *counters = new int[nLevels + 1];
		counters[0] = 0;
		for(int i = 1; i <= nLevels; i++)
			counters[i] = 0;

		run(counters + 1, 0, nLevels);

		delete[] counters;
		onEndOptimizing(configuration, gameRunner);
	}
protected:
	virtual int getNLevels() = 0;
	virtual int getNIterations(int *iterationsCounter, int level, int nLevels) = 0;
	virtual int createCmaesLevel()
	{
		return 0;
	}

	virtual void onBeginOptimizing(Configuration *configuration, GameRunner *gameRunner) { }
	virtual void onEndOptimizing(Configuration *configuration, GameRunner *gameRunner) { }
	virtual void beforeIteration(int *iterationsCounter, int level, int nLevels) { }
	virtual void afterIteration(int *iterationsCounter, int level, int nLevels) { }
	virtual void onBeginLevel(int *iterationsCounter, int level, int nLevels) { }
	virtual int onEndLevel(int *iterationsCounter, int level, int nLevels) { return level; }
	virtual void onBeginLearning() { }
	virtual void onEndLearning() { }
};

class ValidationCmaEsOptimizer : public AbsCmaEsOptimizer
{
	GameRunner *validationRunner;
	int nEpochs;
protected:
	Board::EVALUATION_TYPE validate(Board::EVALUATION_TYPE *weights)
	{
		return validationRunner->runTest(weights);
	}

	bool hasEpochs()
	{
		return nEpochs > 0;
	}

	int getNEpochs()
	{
		return nEpochs;
	}
public:
	ValidationCmaEsOptimizer(GameRunner *validationRunner, int nEpochs, int seed)
		: AbsCmaEsOptimizer(seed),
		validationRunner(validationRunner),
		nEpochs(nEpochs)
	{
	}

	int getNLevels()
	{
		return hasEpochs() ? 2 : 1;
	}

	int createCmaesLevel()
	{
		return hasEpochs() ? 1 : 0;
	}
};

class LogableOptimizer : public ValidationCmaEsOptimizer
{
	Logger *logger;
protected:
	Logger *getLogger()
	{
		return logger;
	}

	Watch<float> watch;
public:
	LogableOptimizer(GameRunner *validationRunner, int nEpochs, Logger *logger, int seed)
		: ValidationCmaEsOptimizer(validationRunner, nEpochs, seed),
		logger(logger)
	{
		watch.start();
	}
};

class EpsZeroOptimizer : public LogableOptimizer
{
	int nIterations;
	Board::EVALUATION_TYPE playerFreq;
protected:
	virtual Board::EVALUATION_TYPE getFreq(int itCounter) = 0;
	int getNIterations()
	{
		return nIterations;
	}
public:
	EpsZeroOptimizer(GameRunner *validationRunner, int nEpochs, int nIterations, Board::EVALUATION_TYPE playerFreq, Logger *logger, int seed) :
		LogableOptimizer(validationRunner, nEpochs, logger, seed),
		nIterations(nIterations),
		playerFreq(playerFreq)
	{
	}

	int getNIterations(int *iterationsCounter, int level, int nLevels)
	{
		if (hasEpochs())
		{
			if (level == 0)
				return getNEpochs();
			else
				return nIterations / getNEpochs();
		}
		else
		{
			return nIterations;
		}
	}

	void onBeginOptimizing(Configuration *configuration, GameRunner *gameRunner)
	{
		auto logger = getLogger();
		logger->start();
		logger->beginEntry();
		logger->print("Player: %s, %d weights\n", configuration->getPlayerName(0).c_str(), configuration->getPlayerLoader(0)->getNWeights());
		for(int i = 1; i < configuration->getNPlayers(); i++)
		{
			logger->print("Expert: %s\t%d\n", configuration->getPlayerName(i).c_str(), configuration->getPlayerLoader(i)->getNWeights());
		}
		logger->endEntry();
	}

	void beforeIteration(int *iterationsCounter, int level, int nLevels)
	{
		if (hasEpochs())
		{
			if (level == 0)
			{
				currentGameRunner->setPlayerFreq(playerFreq);
				Board::EVALUATION_TYPE freq = getFreq(iterationsCounter[0]);
				currentGameRunner->setExpertsFreq(freq);
			}
		}
		else
		{
			currentGameRunner->setPlayerFreq(playerFreq);
			Board::EVALUATION_TYPE freq = getFreq(iterationsCounter[0]);
			currentGameRunner->setExpertsFreq(freq);
		}

		if (level == 0)
		{
			int nIt = getNIterations(iterationsCounter, level, nLevels);
			int val = 100 * iterationsCounter[0] / nIt;
			printf("%d%%  \r", val);
			fflush(stdout);
		}
	}

	void afterIteration(int *iterationsCounter, int level, int nLevels)
	{
		if (level == nLevels - 1)
		{
			watch.stop();
			auto logger = getLogger();
			logger->beginEntry();

			Board::EVALUATION_TYPE freq = getFreq(iterationsCounter[0]);
			auto weights = getCmaes()->getNew(CMAES<Board::EVALUATION_TYPE>::XBest);
			Board::EVALUATION_TYPE result = currentGameRunner->runTest(weights);
			Board::EVALUATION_TYPE validResult = validate(weights);
			saveWeights(weightsFile, bestWeightsFile, weights, currentConfiguration->getPlayerLoader(0)->getNWeights(), iterationsCounter[-1], iterationsCounter, level, validResult, bestWeightsEvaluation < validResult);
			if (bestWeightsEvaluation < validResult)
			{
				bestWeightsEvaluation = validResult;
				delete[] bestWeights;
				bestWeights = weights;
			}
			else
			{
				delete[] weights;
			}

			int nGames = (currentConfiguration->getNPlayers() - 1) * currentConfiguration->getBoards()->getNBoards() * currentGameRunner->getPreferedNWeights();
			logger->print("%f\t%f\t%f\t%f\n", freq, result, validResult, nGames / watch());

			logger->endEntry();
			watch.start();
		}
	}

	int onEndLevel(int *iterationsCounter, int level, int nLevels)
	{
		return level;
	}

	virtual void onBeginLearning()
	{
	}
};

class VariableEpsOptimizer : public EpsZeroOptimizer
{
protected:
	virtual Board::EVALUATION_TYPE getFreq(int itCounter)
	{
		if (hasEpochs())
			return 1 - powf(itCounter / (float)(getNEpochs() - 1), .35f);
		else
			return 1 - powf(itCounter / (float)(getNIterations() - 1), .35f);
	}
public:
	VariableEpsOptimizer(GameRunner *validationRunner, int nEpochs, int nIterations, Board::EVALUATION_TYPE playerFreq, Logger *logger, int seed) :
	  EpsZeroOptimizer(validationRunner, nEpochs, nIterations, playerFreq, logger, seed)
	{
	}
};

class ConstantEpsOptimizer : public EpsZeroOptimizer
{
	Board::EVALUATION_TYPE expertsFreq;
protected:
	virtual Board::EVALUATION_TYPE getFreq(int itCounter)
	{
		return expertsFreq;
	}

	bool continueLearning()
	{
	    printf("Continue2\n");
	    return true;
	}
public:
	ConstantEpsOptimizer(GameRunner *validationRunner, int nIterations, Board::EVALUATION_TYPE expertsFreq, Board::EVALUATION_TYPE playerFreq, Logger *logger, int seed) :
		EpsZeroOptimizer(validationRunner, 0, nIterations, playerFreq, logger, seed),
		expertsFreq(expertsFreq)
	{
	}
};

#endif //CMA_ES_OPTIMIZER
