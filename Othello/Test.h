#ifndef TEST_H
#define TEST_H

#ifndef NOT_TH
#include <thread>
#include <mutex>
#endif // NOT_TH
#include <iostream>
#include <iomanip>
#include "Watch.h"
#include "Othello.h"
#include "OthelloPlayer.h"
#include "Random.h"
#include "cma-es/cmaes.h"
#include "WeightsOptimizer.h"
#include "CpuGameRunner.h"
#include "CmaEsOptimizer.h"

class Test
{
public:
	static void boardThreaded(int min, int max, int N, Othello *othello, BoardLoader *boardLoader, Board::EVALUATION_TYPE **results, int N_GAMES, int seed
       #ifndef NOT_TH
       , std::mutex *mtx
       #endif // NOT_TH
       )
	{
		Rand r(seed);

		for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			if (i == j)
				continue;
			for (int b = min; b < max; b++)
			{
				Board board(boardLoader->getBoardValues(b));
				for (int k = 0; k < N_GAMES; k++)
				{
					Board tmpBoard(board);
					auto res = othello->play(&tmpBoard, i, j);
					#ifndef NOT_TH
                    mtx->lock();
                    #endif // NOT_TH
					results[i][j] += res.item1;
					#ifndef NOT_TH
                    mtx->unlock();
                    #endif // NOT_TH
				}
			}
		}
	}

	static void boardTest(const char *configuration, int seed, int N_THREADS, float freq)
	{
		Rand r(seed);
		auto conf = Configuration::getConf(configuration, r.rand());
		int N = conf->getNPlayers();

		int gamesCounter = 0;
		OthelloPlayer **players = new OthelloPlayer*[N];
		for (int i = 0; i < N; i++)
		{
			players[i] = conf->getPlayerLoader(i)->getPlayer(r.rand(), conf->getPlayerNeg(i));
			players[i]->setRandomMoveFreq(freq);
		}

		const int N_GAMES = 1;

		Board::EVALUATION_TYPE **results = new Board::EVALUATION_TYPE*[N];
		for (int i = 0; i < N; i++)
		{
			results[i] = new Board::EVALUATION_TYPE[N];
			for (int j = 0; j < N; j++)
				results[i][j] = 0;
		}

		Watch<float> watch;
#ifndef NOT_TH
		std::thread **th = new std::thread*[N_THREADS];
		Othello **othellos = new Othello*[N_THREADS];
		int nBoards = conf->getBoards()->getNBoards();
		std::mutex m;
		for (int i = 0; i < N_THREADS; i++)
		{
			int min = (int)((i / (float)N_THREADS) * nBoards);
			int max = (int)(((i + 1) / (float)N_THREADS) * nBoards);
			othellos[i] = new Othello(players, N, r.rand());
			th[i] = new std::thread(boardThreaded, min, max, N, othellos[i], conf->getBoards(), results, N_GAMES, r.rand(), &m);
		}
		for (int i = 0; i < N_THREADS; i++)
		{
			th[i]->join();
			delete th[i];
			delete othellos[i];
		}
		delete[] othellos;
		delete[] th;
#else
		auto oth = new Othello(players, N, 0);
		for (int i = 0; i < N_THREADS; i++)
		{
			int nBoards = conf->getBoards()->getNBoards();
			int min = i * ((nBoards + N_THREADS - 1) / (float)N_THREADS);
			int max = (i+1) * ((nBoards + N_THREADS - 1) / (float)N_THREADS);
			boardThreaded(min, max, N, oth, conf->getBoards(), results, N_GAMES, r.rand());
		}
		delete oth;
#endif // TH
		watch.stop();

		int games = N_GAMES * conf->getBoards()->getNBoards();
		printf("GPS: %f\n", games * (N - 1) * N / watch());
		for (int i = 0; i < N; i++)
		{
			float val = 0;
			for (int j = 0; j < N; j++)
				if (i != j)
				{
				    float tmp = results[i][j] / games;
				    tmp += 1 - results[j][i] / games;
				    val += tmp / 2;
				}
			val /= N - 1;
			if (i < N)
			{
				std::string s(conf->getPlayerName(i));
				s = s.substr(s.find_last_of('/') + 1);
				printf("%-20s\t%.5f\n", s.c_str(), val);
			}
			else
				printf("%.5f\n", val);
		}

		for(int i = 0; i < N; i++)
		{
			for(int j = 0; j < N; j++)
			{
				printf("%.2f  ", results[i][j]/(N_GAMES * conf->getBoards()->getNBoards()));
			}
			printf("\n");
		}

		for (int i = 0; i < N; i++)
			delete[] results[i];
		delete[] results;

		for (int i = 0; i < N; i++)
			delete players[i];
		delete[] players;
		delete conf;
	}

    static void playersThreaded(int min, int max, int N_VALIDS, Othello *othello, BoardLoader *boardLoader, Board::EVALUATION_TYPE *results, int N_GAMES, int seed
       #ifndef NOT_TH
       , std::mutex *mtx
       #endif // NOT_TH
       )
	{
		Rand r(seed);

		for (int i = 0; i < N_VALIDS; i++)
		{
			for (int b = min; b < max; b++)
			{
				Board board(boardLoader->getBoardValues(b));
				for (int k = 0; k < N_GAMES; k++)
				{
					Board tmpBoard(board);
					auto res = othello->playDouble(&tmpBoard, 0, i + 1);
					#ifndef NOT_TH
                    mtx->lock();
                    #endif // NOT_TH
					results[i] += res.item1;
					#ifndef NOT_TH
                    mtx->unlock();
                    #endif // NOT_TH
				}
			}
		}
	}

	static void playersTest(const char *playersConfiguration, const char *validConfiguration, int seed, int N_THREADS, float freq)
	{
		Rand r(seed);
		auto playersConf = Configuration::getConf(playersConfiguration, r.rand());
		auto validConf = Configuration::getConf(validConfiguration, r.rand());
		int N_PLAYERS = playersConf->getNPlayers();
		int N_VALIDS = validConf->getNPlayers();

		OthelloPlayer **players = new OthelloPlayer*[N_PLAYERS];
		for (int i = 0; i < N_PLAYERS; i++)
		{
			players[i] = playersConf->getPlayerLoader(i)->getPlayer(r.rand(), playersConf->getPlayerNeg(i));
			players[i]->setRandomMoveFreq(freq);
		}
		/*for (int i = 0; i < N_PLAYERS; i++)
		{
			int nW = players[i]->getNWeights();
			Board::EVALUATION_TYPE *vals = new Board::EVALUATION_TYPE[nW];
			players[i]->getWeights(vals);
			for(int j = 0; j < nW; j++)
                printf("%d %d:\t%f\n", nW, j, vals[j]);
            delete[] vals;
            getchar();
		}*/
		OthelloPlayer **valids = new OthelloPlayer*[N_VALIDS];
		for (int i = 0; i < N_VALIDS; i++)
		{
			valids[i] = validConf->getPlayerLoader(i)->getPlayer(r.rand(), validConf->getPlayerNeg(i));
			valids[i]->setRandomMoveFreq(freq);
		}

		const int N_GAMES = 1;

		Board::EVALUATION_TYPE **results = new Board::EVALUATION_TYPE*[N_PLAYERS];
		for (int i = 0; i < N_PLAYERS; i++)
		{
			results[i] = new Board::EVALUATION_TYPE[N_VALIDS];
			for (int j = 0; j < N_VALIDS; j++)
				results[i][j] = 0;
		}

		Watch<float> watch;
		for(int p = 0; p < N_PLAYERS; p++)
        {
#ifndef NOT_TH
            std::thread **th = new std::thread*[N_THREADS];
            Othello **othellos = new Othello*[N_THREADS];
            int nBoards = validConf->getBoards()->getNBoards();
            std::mutex m;
            for (int i = 0; i < N_THREADS; i++)
            {
                int min = (int)((i / (float)N_THREADS) * nBoards);
                int max = (int)(((i + 1) / (float)N_THREADS) * nBoards);
                othellos[i] = new Othello(players[p], valids, N_VALIDS, r.rand());
                th[i] = new std::thread(playersThreaded, min, max, N_VALIDS, othellos[i], validConf->getBoards(), results[p], N_GAMES, r.rand(), &m);
            }
            for (int i = 0; i < N_THREADS; i++)
            {
                th[i]->join();
                delete th[i];
                delete othellos[i];
            }
            delete[] othellos;
            delete[] th;
#else
            auto othello = new Othello(players[p], valids, N_VALIDS, seed);
            int nBoards = validConf->getBoards()->getNBoards();
            for (int i = 0; i < N_THREADS; i++)
            {
                int min = (int)((i / (float)N_THREADS) * nBoards);
                int max = (int)(((i + 1) / (float)N_THREADS) * nBoards);
                playersThreaded(min, max, N_VALIDS, othello, validConf->getBoards(), results[p], N_GAMES, r.rand());
            }
			delete othello;
#endif // NOT_TH
        }
		watch.stop();

		int games = N_GAMES * validConf->getBoards()->getNBoards() * N_VALIDS;
		printf("GPS: %f\n", games * N_PLAYERS / watch());
		for (int i = 0; i < N_PLAYERS; i++)
		{
			float val = 0;
			for (int j = 0; j < N_VALIDS; j++)
                val += results[i][j] / games;

			std::string s(playersConf->getPlayerName(i));
            //s = s.substr(s.find_last_of('/') + 1);
            printf("%-20s\t%.5f\n", s.c_str(), val);
		}

		for (int i = 0; i < N_PLAYERS; i++)
			delete[] results[i];
		delete[] results;

        for (int i = 0; i < N_VALIDS; i++)
			delete valids[i];
		delete[] valids;

		for (int i = 0; i < N_PLAYERS; i++)
			delete players[i];
		delete[] players;
		delete validConf;
		delete playersConf;
	}

	static void performanceTest(const char *configuration, int seed, int N_THREADS)
	{
		Rand r(seed);
		auto conf = Configuration::getConf(configuration, r.rand());
		int N = conf->getNPlayers();

		int gamesCounter = 0;
		OthelloPlayer **players = new OthelloPlayer*[N];
		for (int i = 0; i < N; i++)
		{
			players[i] = conf->getPlayerLoader(i)->getPlayer(r.rand(), conf->getPlayerNeg(i));
			players[i]->setRandomMoveFreq(0);
		}

		const int N_GAMES = 1;

		Board::EVALUATION_TYPE **results = new Board::EVALUATION_TYPE*[N];
		for (int i = 0; i < N; i++)
		{
			results[i] = new Board::EVALUATION_TYPE[N];
			for (int j = 0; j < N; j++)
				results[i][j] = 0;
		}

		Watch<float> watch;
#ifndef NOT_TH
		std::thread **th = new std::thread*[N_THREADS];
		Othello **othellos = new Othello*[N_THREADS];
		int nBoards = conf->getBoards()->getNBoards();
		std::mutex m;
		for (int i = 0; i < N_THREADS; i++)
		{
			int min = 0;
			int max = nBoards;
			othellos[i] = new Othello(players, N, i);
			th[i] = new std::thread(boardThreaded, min, max, N, othellos[i], conf->getBoards(), results, N_GAMES, r.rand(), &m);
		}
		for (int i = 0; i < N_THREADS; i++)
		{
			th[i]->join();
			delete th[i];
			delete othellos[i];
		}
		delete[] othellos;
		delete[] th;
#else
		auto oth = new Othello(players, N, 0);
		for (int i = 0; i < N_THREADS; i++)
		{
			int nBoards = conf->getBoards()->getNBoards();
			int min = i * ((nBoards + N_THREADS - 1) / (float)N_THREADS);
			int max = (i+1) * ((nBoards + N_THREADS - 1) / (float)N_THREADS);
			boardThreaded(min, max, N, oth, conf->getBoards(), results, N_GAMES, r.rand());
		}
		delete oth;
#endif // TH
		watch.stop();

		int games = N_THREADS * N_GAMES * conf->getBoards()->getNBoards();
		printf("GPS: %f\n", games * (N - 1) * N / watch());

		for (int i = 0; i < N; i++)
			delete[] results[i];
		delete[] results;

		for (int i = 0; i < N; i++)
			delete players[i];
		delete[] players;
		delete conf;
	}

	static void threadedLearner(Board::EVALUATION_TYPE *res, OthelloPlayer *player, Othello *othello, int nExperts, Board::EVALUATION_TYPE *const*pop, int nWeights, UniversalLoader *boardLoader)
	{
		const int N_GAMES = 1;

		for (int p = 0; p < nWeights; ++p)
		{
		    player->setWeights(pop[p]);
			Board::EVALUATION_TYPE result = 0;
			int games = nExperts * boardLoader->getNValues();
			for (int i = 0; i < nExperts; i++)
			{
				int nBoards = boardLoader->getNValues();
				for (int b = 0; b < nBoards; b++)
				{
					Board board(boardLoader->getValues() + b * 64);
					for (int k = 0; k < N_GAMES; k++)
					{
						Board tmpBoard(board);
						auto res = othello->playDouble(&tmpBoard, 0, i + 1);
						result += res.item1;
					}
				}
			}
			result /= games;
			res[p] = 1 - result;
		}
	}

	static void learner2(AbsCmaEsOptimizer *optimizer, const char *configFile, const char *logFile, int nThreads, int nGames, int seed)
	{
		Rand r(seed);

		auto conf = Configuration::getConf(configFile, r.rand());
		auto gameRunner = new SimpleCPUGameRunner(nThreads, nGames, r.rand());
		auto logger = new TxtLogger(logFile, conf, false);

		optimizer->optimize(conf, gameRunner);

        delete logger;
		delete gameRunner;
		delete conf;
	}

private:
#ifndef NOT_TH
    template<typename P1, typename P2 = P1>
#else
	template<typename P1, typename P2>
#endif
    static void playGame(int *retCounter, float playTime)
    {
        P1 player1(rand());
        P2 player2(rand());

        Watch<float> w;

        int counter = 0;

        Board::EVALUATION_TYPE r1(0), r2(0);
        while (w.current() < playTime)
        {
            Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result = Othello::playDouble(&player1, &player2);
            r1 += result.item1;
            r2 += result.item2;
            counter++;
        }

        if (retCounter != 0)
            *retCounter = counter;
    }

#ifndef NOT_TH
    template<typename P1, typename P2 = P1>
#else
	template<typename P1, typename P2>
#endif
    static void runThreaded(int nThreads, float playTime = 1)
    {
        Watch<float> w;
        std::thread **threads = 0;
        int *counters = 0;
        if (nThreads > 0)
        {
            threads = new std::thread*[nThreads];
            counters = new int[nThreads];
        }
        for (int t = 0; t < nThreads; t++)
        {
            threads[t] = new std::thread(playGame<P1, P2>, &counters[t], playTime);
        }
        int totalCounter = 0;
        if (nThreads == 0)
        {
            playGame<P1, P2>(&totalCounter, playTime);
        }
        for (int t = 0; t < nThreads; t++)
        {
            threads[t]->join();
            delete threads[t];
            totalCounter += counters[t];
        }
        w.stop();
        float time = w();
        std::cout << std::setw(6) << nThreads << ":"
                    << std::setw(6) << totalCounter
                    << std::setw(6)<< int(time * 1000)
                    << std::setw(8) << int(totalCounter / time + .5)
                    << std::endl;

        if (nThreads > 0)
        {
            delete[] counters;
            delete[] threads;
        }
    }
};

#endif //TEST_H
