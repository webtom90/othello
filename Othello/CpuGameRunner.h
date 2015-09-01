#ifndef CPU_GAME_RUNNER
#define CPU_GAME_RUNNER

#include "Othello.h"
#include "WeightsOptimizer.h"

class CPUGameRunner : public GameRunner
{
protected:
	Rand rand;
	int nThreads;
	int perThread;

	OthelloPlayer **players;
	OthelloPlayer **experts;
	Othello **othellos;
	int nExperts;

	void play(OthelloPlayer *player, Othello *othello, Board::EVALUATION_TYPE *const*weights, int nWeights, Board::EVALUATION_TYPE *results, int minBoard, int maxBoard)
	{
		auto boards = conf->getBoards();
		int nBoards = boards->getNBoards();

		for (int i = 0; i < nWeights; i++)
		{
			if (weights[i] != nullptr)
				player->setWeights(weights[i]);
			Board::EVALUATION_TYPE result = 0;
			int games = nBoards * nExperts;
            for (int b = 0; b < nBoards; b++)
			{
                Board board(boards->getBoardValues(b));
                for (int e = 0; e < nExperts; e++)
				{
				    Board b(board);
				    auto res = othello->playDouble(&b, 0, e+1);
					//auto res = Othello::playDouble(b, player, experts[i], i);
					result += res.item1;
				}
			}
			result /= games;
			results[i] = 1 - result;
		}
	}

	void static runThreaded(CPUGameRunner *runner, OthelloPlayer *player, Othello *othello, Board::EVALUATION_TYPE *const*weights, int nWeights, Board::EVALUATION_TYPE *results)
	{
		runner->play(player, othello, weights, nWeights, results, 0, runner->conf->getBoards()->getNBoards());
	}

	void clear()
	{
	    if (othellos != nullptr)
        {
            for (int i = 0; i < nThreads; i++)
				delete othellos[i];
			delete[] othellos;
			othellos = nullptr;
        }
		if (players != nullptr)
		{
			for (int i = 0; i < nThreads; i++)
				delete players[i];
			delete[] players;
			players = nullptr;
		}
		if (experts != nullptr)
		{
			for (int i = 0; i < nExperts; i++)
				delete experts[i];
			delete[] experts;
			nExperts = 0;
			experts = nullptr;
		}
	}

	virtual void _setPlayerFreq(float freq)
	{
		for (int i = 0; i < nThreads; i++)
			players[i]->setRandomMoveFreq(freq);
	}

	virtual void _setExpertsFreq(float freq)
	{
		for (int i = 0; i < nExperts; i++)
			experts[i]->setRandomMoveFreq(freq);
	}
public:
	CPUGameRunner(int nThreads, int perThread, int seed) :
		rand(seed),
		experts(nullptr),
		nExperts(0),
		players(nullptr),
		othellos(nullptr),
		nThreads(nThreads),
		perThread(perThread)
	{
	}

	virtual ~CPUGameRunner()
	{
		clear();
	}

	bool init(Configuration *conf)
	{
		clear();
		this->conf = conf;

		players = new OthelloPlayer*[nThreads];
		for (int i = 0; i < nThreads; i++)
		{
			players[i] = conf->getPlayerLoader(0)->getPlayer(rand.rand(), conf->getPlayerNeg(0));
			if (players[i] == nullptr)
			{
				//printf("Cannot load player %d %d %d %d %d\n", i, conf->getPlayerLoader()->getNFields(), conf->getPlayer()->getNWeights(), conf->getPlayer()->getNTuples(), conf->getPlayer()->getMaxTuplePerPos());
				printf("Cannot load player %s\n", conf->getPlayerName(i).c_str());
				return false;
			}
		}

		nExperts = conf->getNPlayers() - 1;
		experts = new OthelloPlayer*[nExperts];
		for (int i = 0; i < nExperts; i++)
			experts[i] = conf->getPlayerLoader(i + 1)->getPlayer(rand.rand(), conf->getPlayerNeg(i + 1));

        othellos = new Othello*[nThreads];
        for (int i = 0; i < nThreads; i++)
        {
            othellos[i] = new Othello(players[i], experts, nExperts, rand.rand());
        }

		return true;
	}

	int getNPlayers()
	{
		return nThreads;
	}

	int getPreferedNWeights()
	{
		return nThreads * perThread;
	}

	int getMinimumNWeights()
	{
		return nThreads;
	}
};

class SimpleCPUGameRunner : public CPUGameRunner
{
public:
	SimpleCPUGameRunner(int nThreads, int perThread, int seed) :
		CPUGameRunner(nThreads, perThread, seed)
	{
	}

	bool run(Board::EVALUATION_TYPE *const*weights, int nWeights, Board::EVALUATION_TYPE *results)
	{
		if (nThreads * perThread == nWeights || nThreads == nWeights)
		{
#ifndef NOT_TH
			int perTh = nWeights / nThreads;
			std::thread **th = new std::thread*[nThreads];
			for (int i = 0; i < nThreads; i++)
			{
				auto p = players[i];
				int index = i * perTh;
				th[i] = new std::thread(&CPUGameRunner::runThreaded, this, p, othellos[i], &weights[index], perTh, &results[index]);
			}

			for (int i = 0; i < nThreads; i++)
				th[i]->join();

			for (int i = 0; i < nThreads; i++)
				delete th[i];

			delete[] th;
#else
			int perTh = nWeights / nThreads;
			for (int i = 0; i < nThreads; i++)
			{
				auto p = players[i];
				int index = i * perTh;
				runThreaded(this, p, othellos[0], &weights[index], perTh, &results[index]);
			}
#endif // TH
			return true;
		}
		else if (nWeights == 1)
		{
			runThreaded(this, players[0], othellos[0], weights, 1, results);
			return true;
		}
		else
		{
			printf("Niepoprawna ilosc wag: %d zamiast %d lub %d\n", nWeights, nThreads * perThread, nThreads);
			return false;
		}
	}
};

#endif //CPU_GAME_RUNNER
