#define DEF

#include <stdio.h>
#include "Header.h"
#include "Test.h"
#include "TupleLoader.h"
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif // defined

const int N_EPOCHS = 20;

template<int val> AbsCmaEsOptimizer *getCOpt(int N_ITERATIONS, Logger *logger, Rand &r, SimpleCPUGameRunner *vgameRunner)
{
    return new ConstantEpsOptimizer(vgameRunner, N_ITERATIONS, val / 100.0f, val / 100.0f, logger, r.rand());
}

template<int val> AbsCmaEsOptimizer *getOpt(int N_ITERATIONS, Logger *logger, Rand &r, SimpleCPUGameRunner *vgameRunner)
{
    return new VariableEpsOptimizer(vgameRunner, N_EPOCHS, N_ITERATIONS, val / 100.0f, logger, r.rand());
}

void run(int argc, const char *argv[], AbsCmaEsOptimizer *(*f)(int N_ITERATIONS, Logger *logger, Rand &r, SimpleCPUGameRunner *vgameRunner))
{
    if (argc != 10)
    {
        printf("9 parameters needed\n");
        printf("type, config, valid_config, log, weights, bestWeights, nThreads, nGames, seed\n");
        return;
    }

    auto conf = argv[2];
    auto validConf = argv[3];
    auto log = argv[4];
    auto weights = argv[5];
    auto best = argv[6];
    auto nTh = atoi(argv[7]);
    auto nGames = atoi(argv[8]);
    auto seed = atoi(argv[9]);

    auto vconf = Configuration::getConf(validConf, 0);
    auto vgameRunner = new SimpleCPUGameRunner(nTh, nGames, 0);
    vgameRunner->init(vconf);

    Rand r(seed);

    auto config = Configuration::getConf(conf, r.rand());
    auto gameRunner = new SimpleCPUGameRunner(nTh, nGames, r.rand());
    auto logger = new TxtLogger(log, config, false);

    int nIt = 24 * 4 * 4000 / (nGames * nTh);
    AbsCmaEsOptimizer *opt = f(nIt, logger, r, vgameRunner);
    opt->setFiles(weights, best);

    Watch<float> w;
    w.start();

    Test::learner2(opt, conf, log, nTh, nGames, seed);

    w.stop();
    printf("Total time: %f\n", w());

    delete logger;
    delete gameRunner;
    delete config;
    delete vgameRunner;
    delete vconf;
}

void runFunc(const char *path, int seed)
{
    Rand r(seed);
    Configuration *conf = Configuration::getConf(path, r.rand());
    Watch<float> watch;
    #define LOOP_
    #ifndef LOOP
    OthelloPlayer **players = new OthelloPlayer*[conf->getNPlayers()];
    for(int i = 0; i < conf->getNPlayers(); i++)
        players[i] = conf->getPlayerLoader(i)->getPlayer(0, conf->getPlayerNeg(i));
    Othello oth(players, conf->getNPlayers(), r.rand());
    #endif // LOOP
    for(int i = 1; i < conf->getNPlayers(); i++)
    {
        for(int b = 0; b < conf->getBoards()->getNBoards(); b++)
        {
            Board brd(conf->getBoards()->getBoardValues(b));
            #ifdef LOOP
            OthelloPlayer *p1 = conf->getPlayerLoader(0)->getPlayer(r.rand(), conf->getPlayerNeg(0));
            OthelloPlayer *p2 = conf->getPlayerLoader(i)->getPlayer(r.rand(), conf->getPlayerNeg(i));
            Othello::playDouble(brd, p1, p2, r.rand());
            delete p1;
            delete p2;
            #else
            oth.playDouble(&brd, 0, i);
            #endif // LOOP
        }
    }

    #ifndef LOOP
    for(int i = 0; i < conf->getNPlayers(); i++)
        delete players[i];
    delete[] players;
    #endif // LOOP

    watch.stop();
    int gps = int(2 * (conf->getNPlayers() - 1) * conf->getBoards()->getNBoards() / watch());
    printf("%d\n", gps);
    delete conf;
}

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        runFunc(argv[1], 0);
        return 0;
    }

    #if defined(WIN32) || defined(_WIN32)
    if(!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS))
        printf("Could not set priority\n");
    #endif // defined

    if (argc < 2)
    {
        printf("Need type parameter\n");
        printf("learn_start|learn_continue|test|performance|convert\n");
        return 0;
    }
    if (strcmp(argv[1], "learn_0") == 0)
    {
        run(argc, argv, getOpt<0>);
    }
    else if (strcmp(argv[1], "learn_1") == 0)
    {
        run(argc, argv, getOpt<1>);
    }
    else if (strcmp(argv[1], "learn_2") == 0)
    {
        run(argc, argv, getOpt<2>);
    }
    else if (strcmp(argv[1], "learn_3") == 0)
    {
        run(argc, argv, getOpt<3>);
    }
    else if (strcmp(argv[1], "learn_4") == 0)
    {
        run(argc, argv, getOpt<4>);
    }
    else if (strcmp(argv[1], "learn_5") == 0)
    {
        run(argc, argv, getOpt<5>);
    }
    else if (strcmp(argv[1], "learn_6") == 0)
    {
        run(argc, argv, getOpt<6>);
    }
    else if (strcmp(argv[1], "learn_7") == 0)
    {
        run(argc, argv, getOpt<7>);
    }
    else if (strcmp(argv[1], "learn_8") == 0)
    {
        run(argc, argv, getOpt<8>);
    }
    else if (strcmp(argv[1], "learn_9") == 0)
    {
        run(argc, argv, getOpt<9>);
    }
    else if (strcmp(argv[1], "learn_10") == 0)
    {
        run(argc, argv, getOpt<10>);
    }
    else if (strcmp(argv[1], "clearn_0") == 0)
    {
        run(argc, argv, getCOpt<0>);
    }
    else if (strcmp(argv[1], "clearn_3") == 0)
    {
        run(argc, argv, getCOpt<3>);
    }
    else if (strcmp(argv[1], "clearn_5") == 0)
    {
        run(argc, argv, getCOpt<5>);
    }
    else if (strcmp(argv[1], "clearn_10") == 0)
    {
        run(argc, argv, getCOpt<10>);
    }
    else if (strcmp(argv[1], "ctest") == 0 || strcmp(argv[1], "ctestR") == 0 || strcmp(argv[1], "ctestN") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed\n");
            return 0;
        }
        auto confS = argv[2];
        auto seed = atoi(argv[3]);
        int nIt = 1;
        if (argc > 4)
            nIt = atoi(argv[4]);

        Rand r(seed);

		auto conf = Configuration::getConf(confS, r.rand());
		int N = conf->getNPlayers();
		printf("Players: %d\n", N);

		OthelloPlayer **players = new OthelloPlayer*[N];
		for (int i = 0; i < N; i++)
		{
			players[i] = conf->getPlayerLoader(i)->getPlayer(r.rand(), conf->getPlayerNeg(i));
			players[i]->setRandomMoveFreq(0);
		}

        if (strcmp(argv[1], "ctestR") == 0)
        {
            int seed2 = 0;
            if (argc > 5)
                seed2 = atoi(argv[5]);
            Random<Board::EVALUATION_TYPE> rF(seed2);

            for (int i = 0; i < N; i++)
            {
                Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[players[i]->getNWeights()];
                for(int j = 0; j < players[i]->getNWeights(); j++)
                    weights[j] = rF.getValue(0, 1);
                players[i]->setWeights(weights);
                delete[] weights;
            }
        }

        if (strcmp(argv[1], "ctestN") == 0)
        {
            for (int i = 0; i < N; i++)
            {
                Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[players[i]->getNWeights()];
                for(int j = 0; j < players[i]->getNWeights(); j++)
                    weights[j] = j;
                players[i]->setWeights(weights);
                delete[] weights;
            }
        }

		Watch<float> w;

		for(int i = 0; i < N; i++)
        {
            for(int j = 0; j < N; j++)
            {
                if (i == j)
                    continue;

                for(int it = 0; it < nIt; it++)
                for(int b = 0; b < conf->getBoards()->getNBoards(); b++)
                {
                    Board board(conf->getBoards()->getBoardValues(b));
                    Othello::play(board, players[i], players[j], r.rand());
                }
            }
        }

        w.stop();
        printf("%f, %d %d\n", (float)N * (N - 1) * conf->getBoards()->getNBoards() * nIt / w(), N, nIt);

        for (int i = 0; i < N; i++)
            delete players[i];
        delete[] players;
        delete conf;
    }
    else if (strcmp(argv[1], "ctest2") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed\n");
            return 0;
        }
        auto confS = argv[2];
        auto seed = atoi(argv[3]);
        int nIt = 1;
        if (argc > 4)
            nIt = atoi(argv[4]);

        Rand r(seed);

		auto conf = Configuration::getConf(confS, r.rand());
		int N = conf->getNPlayers();

		Watch<float> w;

		for(int i = 0; i < N; i++)
        {
            for(int j = 0; j < N; j++)
            {
                if (i == j)
                    continue;

                for(int it = 0; it < nIt; it++)
                for(int b = 0; b < conf->getBoards()->getNBoards(); b++)
                {
                    Board board(conf->getBoards()->getBoardValues(b));

                    auto p1 = conf->getPlayerLoader(i)->getPlayer(r.rand(), conf->getPlayerNeg(i));
                    p1->setRandomMoveFreq(0);
                    auto p2 = conf->getPlayerLoader(j)->getPlayer(r.rand(), conf->getPlayerNeg(j));
                    p2->setRandomMoveFreq(0);

                    Othello::play(board, p1, p2, r.rand());

                    delete p1;
                    delete p2;
                }
            }
        }

        w.stop();
        printf("%f, %d %d\n", (float)N * (N - 1) * conf->getBoards()->getNBoards() * nIt / w(), N, nIt);
        delete conf;
    }
    else if (strcmp(argv[1], "test") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed (, nThreads=8)\n");
            return 0;
        }
        auto conf = argv[2];
        auto seed = atoi(argv[3]);
        int nThreads = 8;
        if (argc > 4)
            nThreads = atoi(argv[4]);
        Test::boardTest(conf, seed, nThreads, 0);
    }
    else if (strcmp(argv[1], "test05") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed (, nThreads=8)\n");
            return 0;
        }
        auto conf = argv[2];
        auto seed = atoi(argv[3]);
        int nThreads = 8;
        if (argc > 4)
            nThreads = atoi(argv[4]);
        Test::boardTest(conf, seed, nThreads, 0.05);
    }
    else if (strcmp(argv[1], "test1") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed (, nThreads=8)\n");
            return 0;
        }
        auto conf = argv[2];
        auto seed = atoi(argv[3]);
        int nThreads = 8;
        if (argc > 4)
            nThreads = atoi(argv[4]);
        Test::boardTest(conf, seed, nThreads, 0.1);
    }
    else if (strcmp(argv[1], "test_players") == 0)
    {
        if (argc < 5)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config_players, config_valid, seed (, nThreads=8)\n");
            return 0;
        }
        auto playersConf = argv[2];
        auto validConf = argv[3];
        auto seed = atoi(argv[4]);
        int nThreads = 8;
        if (argc > 5)
            nThreads = atoi(argv[5]);
        Test::playersTest(playersConf, validConf, seed, nThreads, 0);
    }
    else if (strcmp(argv[1], "test_players05") == 0)
    {
        if (argc < 5)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config_players, config_valid, seed (, nThreads=8)\n");
            return 0;
        }
        auto playersConf = argv[2];
        auto validConf = argv[3];
        auto seed = atoi(argv[4]);
        int nThreads = 8;
        if (argc > 5)
            nThreads = atoi(argv[5]);
        Test::playersTest(playersConf, validConf, seed, nThreads, 0.05);
    }
    else if (strcmp(argv[1], "test_players1") == 0)
    {
        if (argc < 5)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config_players, config_valid, seed (, nThreads=8)\n");
            return 0;
        }
        auto playersConf = argv[2];
        auto validConf = argv[3];
        auto seed = atoi(argv[4]);
        int nThreads = 8;
        if (argc > 5)
            nThreads = atoi(argv[5]);
        Test::playersTest(playersConf, validConf, seed, nThreads, 0.1);
    }
    else if (strcmp(argv[1], "performance") == 0)
    {
        if (argc < 4)
        {
            printf("Not less then 3 parameters needed\n");
            printf("type, config, seed (, nThreads=8)\n");
            return 0;
        }
        auto conf = argv[2];
        auto seed = atoi(argv[3]);
        int nThreads = 8;
        if (argc > 4)
            nThreads = atoi(argv[4]);
        Test::performanceTest(conf, seed, nThreads);
    }
    else if (strcmp(argv[1], "convert") == 0)
    {
        if (argc != 5)
        {
            printf("4 parameters needed\n");
            printf("type, weights, player, output\n");
            return -1;
        }

        FILE *file = fopen(argv[2], "r");
        UniversalLoader loader;
        loader.load(argv[3]);
        while (getc(file) != '\n') ;
        auto weights = loader.getWeights();
        for(int i = 0; i < loader.getNWeights(); i++)
        {
            Board::EVALUATION_TYPE val;
            if (fscanf(file, "%f\n", &val) != 1)
                printf("Error at: %d\n", i);
            else
                weights[i] = val;
        }

        loader.save(argv[4]);
        fclose(file);
        printf("Success\n");
    }
    else if (strcmp(argv[1], "convert_multi") == 0)
    {
        if (argc != 4)
        {
            printf("2 parameters needed\n");
            printf("player, weights_output\n");
            return -1;
        }

        auto conf = MultiConfiguration::getConf(argv[2]);
        auto loader = conf->getMulitLoader();
        printf("weights: %d\n", loader->getNWeights());
        loader->saveBinary(argv[3]);

        delete conf;
        delete loader;
    }
    else if (strcmp(argv[1], "multi_from_weights") == 0)
    {
        if (argc != 5)
        {
            printf("4 parameters needed\n");
            printf("type, weights, player, output\n");
            return -1;
        }

        FILE *file = fopen(argv[2], "r");
        auto loader = MultiLoader::getLoader(argv[3]);
        while (getc(file) != '\n') ;
        int n = loader->getNWeights();
        Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[n];
        for(int i = 0; i < n; i++)
        {
            Board::EVALUATION_TYPE val;
            if (fscanf(file, "%f\n", &val) != 1)
                printf("Error at: %d\n", i);
            else
                weights[i] = val;
        }

        loader->setWeights(weights);
        loader->saveBinary(argv[4]);
        fclose(file);
        delete[] weights;
        printf("Success\n");
    }
    else if (strcmp(argv[1], "convert_w") == 0)
    {
        if (argc != 4)
        {
            printf("2 parameters needed\n");
            printf("player, weights_output\n");
            return -1;
        }

        UniversalLoader loader;
        loader.load(argv[2]);
        FILE *file = fopen(argv[3], "w");
        fprintf(file, "%d\n", loader.getNWeights());
        auto weights = loader.getWeights();
        for(int i = 0; i < loader.getNWeights(); i++)
        {
            fprintf(file, "%f\n", weights[i]);
        }

        fclose(file);
        printf("Success\n");
    }
    else if (strcmp(argv[1], "convert_w_multi") == 0)
    {
        if (argc != 4)
        {
            printf("2 parameters needed\n");
            printf("player, weights_output\n");
            return -1;
        }

        auto loader = MultiLoader::getLoader(argv[2]);
        FILE *file = fopen(argv[3], "w");
        int n = loader->getNWeights();
        Board::EVALUATION_TYPE *weights = new Board::EVALUATION_TYPE[n];
        fprintf(file, "%d\n", n);
        loader->getWeights(weights);
        for(int i = 0; i < n; i++)
        {
            fprintf(file, "%f\n", weights[i]);
        }

        delete loader;
        delete[] weights;
        fclose(file);
        printf("Success\n");
    }

    return 0;
}
