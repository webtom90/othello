#ifndef OPTIMIZER_CONFIGURATION_H
#define OPTIMIZER_CONFIGURATION_H

#include <fstream>
#include <vector>
#include "AnyLoader.h"
#include "TupleLoader.h"

class AbsConfiguration
{
protected:
	static void getLine(std::ifstream &file, std::string &str)
	{
	    do
        {
            std::getline(file, str);
        }
		while(str.length() > 0 && str[0] == '#');
	}

    static bool loadPlayers(std::ifstream &file, int *nPlayers, std::string **players, bool **playersNeg, float **playersFreq)
    {
        *nPlayers = 0;
        *players = nullptr;
        *playersNeg = nullptr;
        *playersFreq = nullptr;

        std::vector<std::string> playersList;
        std::vector<int> playersN;
		std::vector<float> playersF;

		float currentFreq = 0.0f;
		std::string freqPrefix = "freq:";
        while(true)
        {
            std::string str;
            getLine(file, str);
            if (str.length() == 0)
                break;
			if (!str.compare(0, freqPrefix.size(), freqPrefix))
			{
				currentFreq = ::atof(str.substr(freqPrefix.size()).c_str());
				continue;
			}

            playersList.push_back(str);
			playersF.push_back(currentFreq);
        }

        if (playersList.size() == 0)
        //if (!file.is_open())
        {
            return false;
        }

        while(true)
        {
            std::string str;
            getLine(file, str);
            if (str.length() == 0)
                break;

            int value = atoi(str.c_str());
            if (value < playersList.size())
                playersN.push_back(value);
        }

        *nPlayers = playersList.size();
		*players = new std::string[*nPlayers];
        *playersNeg = new bool[*nPlayers];
		*playersFreq = new float[*nPlayers];
		for(int i = 0; i < *nPlayers; i++)
        {
            (*players)[i] = playersList[i];
			(*playersFreq)[i] = playersF[i];
            (*playersNeg)[i] = false;
        }
        for(int i = 0; i < playersN.size(); i++)
            (*playersNeg)[playersN[i]] = true;

        return true;
    }

    static bool loadBoard(std::ifstream &file, std::string &boardFilename, int &boardSize, int &boardSeed)
    {
        boardSize = 0;
        boardSeed = 0;

        getLine(file, boardFilename);
        bool isEmpty = boardFilename.compare("empty") == 0;
        {
            std::string str;
            getLine(file, str);
            if (str.length() == 0)
                boardSize = -1;
            else
                boardSize = atoi(str.c_str());
        }
        {
            std::string str;
            getLine(file, str);
            if (str.length() == 0)
                boardSeed = 0;
            else
                boardSeed = atoi(str.c_str());
        }
        if (isEmpty && boardSize <= 0)
            return false;

        if (isEmpty)
            boardFilename = "";

        return true;
    }

    static bool loadFloatValues(std::ifstream &file, int *nValues, float **values)
    {
        *nValues = 0;
        *values = nullptr;

        std::vector<float> valuesList;

		while(true)
        {
            std::string str;
            getLine(file, str);
            if (str.length() == 0)
                break;
			float value = ::atof(str.c_str());
			valuesList.push_back(value);
        }

        if (valuesList.size() == 0)
            return false;

        *nValues = valuesList.size();
        *values = new float[valuesList.size()];
        for(int i = 0; i < valuesList.size(); i++)
            (*values)[i] = valuesList[i];

        return true;
    }
};

class Configuration : AbsConfiguration
{
    Random<float> rand;

	int nPlayers;
	PlayerLoader **players;
	bool *playersNeg;
	float *playersFreq;
	std::string *playersNames;

	BoardLoader *boards;

	Configuration(const char** players, bool *playersNegation, float *playersFreq, int nPlayers, const char *board, int nBoards, int boardSeed, int seed) :
		rand(seed)
	{
		load(players, playersNegation, playersFreq, nPlayers);

        if (board != nullptr)
        {
            boards = BoardLoader::getLoader(board);
			if (nBoards != 0)
			{
				BoardLoader *newBoards = boards->getFewerBoards(nBoards, boardSeed);
				delete boards;
				boards = newBoards;
			}
        }
        else
            boards = BoardLoader::getEmptyBoard(nBoards);
	}

	void clear()
	{
		for(int i = 0; i < nPlayers; i++)
			delete players[i];
		if (players)
			delete[] players;
		if (playersNeg)
			delete[] playersNeg;
		if (playersFreq)
			delete[] playersFreq;
		if (playersNames)
			delete[] playersNames;
		players = nullptr;
		playersNeg = nullptr;
		playersNames = nullptr;

		delete boards;
		boards = nullptr;

		nPlayers = 0;
	}

	void load(const char **players, bool *negation, float *freq, int nPlayers)
	{
		this->nPlayers = nPlayers;
		this->players = new PlayerLoader*[nPlayers];
		this->playersNeg = new bool[nPlayers];
		this->playersFreq = new float[nPlayers];
		this->playersNames = new std::string[nPlayers];
		for (int i = 0; i < nPlayers; i++)
		{
			this->players[i] = PlayerLoader::getLoader(players[i]);
			this->playersNeg[i] = negation[i];
			this->playersFreq[i] = freq[i];
			this->playersNames[i] = players[i];
		}
	}

	static Configuration *loadConf(const std::string &filename, int seed)
	{
        std::ifstream file(filename.c_str(), std::ios::in);
        if (!file.is_open())
        {
            printf("Cannot read file %s\n", filename.c_str());
            return nullptr;
        }

        int nPlayers;
		std::string *playersNames;
        bool *playersN;
		float *playersF;

		if (!loadPlayers(file, &nPlayers, &playersNames, &playersN, &playersF))
        {
            printf("Cannot read players from file %s\n", filename.c_str());
            return nullptr;
        }

        const char **players = new const char *[nPlayers];
        for(int i = 0; i < nPlayers ;i++)
            players[i] = playersNames[i].c_str();

        std::string boardFilename;
        int boardSize;
        int boardSeed;
        if (!loadBoard(file, boardFilename, boardSize, boardSeed))
        {
            printf("Empty board have to has positive board size %s\n", filename.c_str());
            return nullptr;
        }

        file.close();

        Configuration *conf;
		conf = new Configuration(players, playersN, playersF, nPlayers, boardFilename.length() > 0 ? boardFilename.c_str() : nullptr, boardSize, boardSeed, seed);

        delete[] players;
        delete[] playersNames;
        delete[] playersN;
        delete[] playersF;

        return conf;
	}
public:
	~Configuration()
	{
		clear();
	}

	int getNPlayers()
	{
		return nPlayers;
	}

	PlayerLoader *getPlayerLoader(int player)
	{
		return players[player];
	}

	bool getPlayerNeg(int player)
	{
		return playersNeg[player];
	}

	float getPlayerFreq(int player)
	{
		return playersFreq[player];
	}

	const std::string &getPlayerName(int player)
	{
		return playersNames[player];
	}

	static Configuration *getConf(const std::string &filename, int seed)
	{
		return loadConf(filename, seed);
	}

	BoardLoader *getBoards()
	{
		return boards;
	}
};

class MultiConfiguration : AbsConfiguration
{
	int nPlayers;
	PlayerLoader **players;
	bool *playersNeg;
	float *playersFreq;
	float *playersVals;
	std::string *playersNames;

	MultiConfiguration(const char** players, bool *playersNegation, float *playersFreq, float *playersVals, int nPlayers)
	{
		load(players, playersNegation, playersFreq, playersVals, nPlayers);
	}

	void clear()
	{
		for(int i = 0; i < nPlayers; i++)
			delete players[i];
		if (players)
			delete[] players;
		if (playersNeg)
			delete[] playersNeg;
		if (playersFreq)
			delete[] playersFreq;
		if (playersNames)
			delete[] playersNames;
        if (playersVals)
            delete[] playersVals;
		players = nullptr;
		playersNeg = nullptr;
		playersNames = nullptr;
		playersVals = nullptr;

		nPlayers = 0;
	}

	void load(const char **players, bool *negation, float *freq, float *playersVals, int nPlayers)
	{
		this->nPlayers = nPlayers;
		this->players = new PlayerLoader*[nPlayers];
		this->playersNeg = new bool[nPlayers];
		this->playersFreq = new float[nPlayers];
		this->playersVals = new float[nPlayers];
		this->playersNames = new std::string[nPlayers];
		for (int i = 0; i < nPlayers; i++)
		{
			this->players[i] = PlayerLoader::getLoader(players[i]);
			this->playersNeg[i] = negation[i];
			this->playersFreq[i] = freq[i];
			this->playersVals[i] = playersVals[i];
			this->playersNames[i] = players[i];
		}
	}

	static MultiConfiguration *loadConf(const std::string &filename)
	{
        std::ifstream file(filename.c_str(), std::ios::in);
        if (!file.is_open())
        {
            printf("Cannot read file %s\n", filename.c_str());
            return nullptr;
        }

        int nPlayers;
		std::string *playersNames;
        bool *playersN;
		float *playersF;

		if (!loadPlayers(file, &nPlayers, &playersNames, &playersN, &playersF))
        {
            printf("Cannot read players from file %s\n", filename.c_str());
            return nullptr;
        }

        const char **players = new const char *[nPlayers];
        for(int i = 0; i < nPlayers ;i++)
            players[i] = playersNames[i].c_str();

        int nValues;
        float *values;

        if (!loadFloatValues(file, &nValues, &values))
        {
            printf("Cannot read values from file %s\n", filename.c_str());
            return nullptr;
        }

        file.close();

        if (nValues < nPlayers)
        {
            printf("%s: %d values and %d players\n", filename.c_str(), nValues, nPlayers);
            return nullptr;
        }

        MultiConfiguration *conf;
		conf = new MultiConfiguration(players, playersN, playersF, values, nPlayers);

        delete[] values;
        delete[] players;
        delete[] playersNames;
        delete[] playersN;
        delete[] playersF;

        return conf;
	}
public:
	~MultiConfiguration()
	{
		clear();
	}

	int getNPlayers()
	{
		return nPlayers;
	}

	PlayerLoader *getPlayerLoader(int player)
	{
		return players[player];
	}

	bool getPlayerNeg(int player)
	{
		return playersNeg[player];
	}

	float getPlayerFreq(int player)
	{
		return playersFreq[player];
	}

	float getPlayerValue(int player)
	{
	    return playersVals[player];
	}

	const std::string &getPlayerName(int player)
	{
		return playersNames[player];
	}

	static MultiConfiguration *getConf(const std::string &filename)
	{
		return loadConf(filename);
	}

	MultiLoader *getMulitLoader()
	{
	    return MultiLoader::get(nPlayers, players, playersVals, playersNeg);
	}
};

#endif // OPTIMIZER_CONFIGURATION_H
