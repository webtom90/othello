#ifndef ANY_LOADER_H
#define ANY_LOADER_H

#include "OthelloPlayer.h"
#include "BinaryReader.h"

class AnyLoader
{
public:
	DEF static AnyLoader *getLoader(char *data, unsigned dataSize, bool own);
	static AnyLoader *getLoader(std::string filename);
	static AnyLoader *getLoader(BinaryReader &reader);
	static AnyLoader *getLoaderBinary(std::string filename);

	DEF static bool isBoardFormat(int format)
	{
	    return format == BOARD_FORMAT;
	}

	DEF static bool isPlayerFormat(int format)
	{
        return format == WPC_FORMAT
            || format == N_TUPLE_FORMAT
            || format == MULTI_PLAYER_FORMAT;
	}

	DEF void saveBinary(std::string filename)
	{
	    BinWriter::writeToFile(filename, data, size);
	}
protected:
	static const int N_TUPLE_FORMAT = 0;
	static const int WPC_FORMAT = 1;
	static const int BOARD_FORMAT = 2;
	static const int MULTI_PLAYER_FORMAT = 3;
	static const int UNKNOWN_FORMAT = -1;

	DEF AnyLoader(char *data, unsigned size, bool ownData)
	{
		this->data = data;
		this->size = size;
		this->ownData = ownData;
	}
public:
	DEF virtual ~AnyLoader()
	{
		if (ownData)
			delete[] data;
	}
protected:
	DEF unsigned getDataSize()
	{
		return size - sizeof(int);
	}

	template <typename T> DEF T *getData()
	{
		int *ptr = reinterpret_cast<int*>(data);
		return reinterpret_cast<T*>(ptr + 1);
	}

	DEF static int getType(char *data)
	{
		return *reinterpret_cast<int*>(data);
	}
public:
	DEF int getType()
	{
		return *reinterpret_cast<int*>(data);
	}

	DEF char *getRawData()
	{
		return data;
	}

	DEF unsigned getRawDataSize()
	{
		return size;
	}
private:
	char *data;
	unsigned size;
	bool ownData;
};

class BoardLoader : public AnyLoader
{
	friend AnyLoader;
	static const int N_VALUES_PER_BOARD = 65;
public:
	DEF int getNBoards()
	{
		return *(AnyLoader::getData<int>());
	}

	DEF Board::BOARD_ELEMENT_TYPE *getBoardValues(int boardNumber)
	{
		int *intPtr = AnyLoader::getData<int>() + 1;
		Board::BOARD_ELEMENT_TYPE *ptr = reinterpret_cast<Board::BOARD_ELEMENT_TYPE *>(intPtr);
		return ptr + boardNumber * N_VALUES_PER_BOARD;
	}

	DEF Board::BOARD_ELEMENT_TYPE getBoardPawnsNumber(int boardNumber)
	{
		int *intPtr = AnyLoader::getData<int>() + 1;
		Board::BOARD_ELEMENT_TYPE *ptr = reinterpret_cast<Board::BOARD_ELEMENT_TYPE *>(intPtr);
		return *(ptr + (boardNumber + 1) * N_VALUES_PER_BOARD - 1);
	}

	static BoardLoader* getLoader(const std::string &filename)
	{
		BinaryReader reader;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!reader.readFromFile(file))
		{
			printf("Nie moge odczytac pliku: %s\n", filename.c_str());
			return nullptr;
		}
		file.close();

		int type;
		reader.read(type);

		if (!isBoardFormat(type))
			return nullptr;

		return load(reader);
	}

	static BoardLoader* load(BinaryReader &reader)
	{
		int nBoards;
		reader.read(nBoards);
		char *data = getData(nBoards);
		Board::BOARD_ELEMENT_TYPE *tmpData = getFirstBoard(data);
		for(int i = 0; i < nBoards; i++)
		{
			Board::BOARD_ELEMENT_TYPE count(0);
			for(int j = 0; j < 64; j++)
			{
				int value;
				reader.read(value);
				Board::BOARD_ELEMENT_TYPE val = (Board::BOARD_ELEMENT_TYPE)value;
				if (val != Board::EMPTY)
					count++;
				*(tmpData++) = val;
			}
			*(tmpData++) = (Board::BOARD_ELEMENT_TYPE)count;
		}

		return new BoardLoader(data, getSize(nBoards), true);
	}

	DEF static BoardLoader *getEmptyBoard(int nBoards)
	{
		if (nBoards <= 0)
			return nullptr;

		char *data = getData(nBoards);
		Board::BOARD_ELEMENT_TYPE *ptr = getFirstBoard(data);
		for (int i = 0; i < nBoards; i++)
		{
			fillEmpty(ptr);
			ptr += N_VALUES_PER_BOARD;
		}

		return new BoardLoader(data, getSize(nBoards), true);
	}

	DEF BoardLoader *getFewerBoards(int nBoards)
	{
		if (nBoards <= 0)
			return nullptr;

		char *data = getData(nBoards);
		Board::BOARD_ELEMENT_TYPE *ptr = getFirstBoard(data);
		int currentBoards = getNBoards();
		for (int i = 0; i < nBoards && i < currentBoards; i++)
		{
			copyBoard(ptr, getBoardValues(i));
			ptr += N_VALUES_PER_BOARD;
		}
		for(int i = currentBoards; i < nBoards; i++)
		{
			fillEmpty(ptr);
			ptr += N_VALUES_PER_BOARD;
		}

		return new BoardLoader(data, getSize(nBoards), true);
	}

	DEF BoardLoader *getFewerBoards(int nBoards, int seed)
	{
		if (nBoards <= 0)
			return nullptr;

		Rand r(seed);

		char *data = getData(nBoards);
		Board::BOARD_ELEMENT_TYPE *ptr = getFirstBoard(data);
		int currentBoards = getNBoards();
		for (int i = 0; i < nBoards; i++)
		{
			copyBoard(ptr, getBoardValues(r.rand() % currentBoards));
			ptr += N_VALUES_PER_BOARD;
		}

		return new BoardLoader(data, getSize(nBoards), true);
	}
protected:
	DEF BoardLoader(char *data, unsigned size, bool ownData)
		: AnyLoader(data, size, ownData) { }
private:
	DEF static char *getData(int nBoards)
	{
		int totalSize = getSize(nBoards);
		char *data = new char[totalSize];
		{
			int *ptr = reinterpret_cast<int*>(data);
			ptr[0] = BOARD_FORMAT;
			ptr[1] = nBoards;
		}
		return data;
	}

	DEF static unsigned getSize(int nBoards)
	{
		return 2 * sizeof(int) + nBoards * N_VALUES_PER_BOARD * sizeof(Board::BOARD_ELEMENT_TYPE);
	}

	DEF static Board::BOARD_ELEMENT_TYPE *getFirstBoard(char *data)
	{
		int *intPtr = reinterpret_cast<int *>(data);
		return reinterpret_cast<Board::BOARD_ELEMENT_TYPE *>(intPtr + 2);
	}

	DEF static void fillEmpty(Board::BOARD_ELEMENT_TYPE *ptr)
	{
		memset(ptr, 0, N_VALUES_PER_BOARD * sizeof(Board::BOARD_ELEMENT_TYPE));
		ptr[27] = ptr[36] = Board::WHITE;
		ptr[28] = ptr[35] = Board::BLACK;
		ptr[64] = 4;
	}

	DEF static void copyBoard(Board::BOARD_ELEMENT_TYPE *dst, Board::BOARD_ELEMENT_TYPE *src)
	{
		memcpy(dst, src, sizeof(Board::BOARD_ELEMENT_TYPE) * N_VALUES_PER_BOARD);
	}
};

class PlayerLoader : public AnyLoader
{
	friend AnyLoader;
public:
	DEF virtual OthelloPlayer *getPlayer(int seed, bool negated) = 0;
	DEF virtual int getNWeights() = 0;
	DEF virtual void getWeights(Board::EVALUATION_TYPE *weights) = 0;
	DEF virtual void setWeights(Board::EVALUATION_TYPE *weights) = 0;
	DEF static PlayerLoader *getLoader(char *data, unsigned dataSize, bool own);
	static PlayerLoader *getLoader(std::string filename);
	static PlayerLoader *getLoader(BinaryReader &reader);
	static PlayerLoader *getLoaderBinary(std::string filename);
protected:
    DEF PlayerLoader(char *data, unsigned size, bool ownData)
        : AnyLoader(data, size, ownData)
    {
    }
};

class WPCLoader : public PlayerLoader
{
	friend AnyLoader;
	friend PlayerLoader;
	static const int N_WEIGHTS = 64;

	DEF Board::EVALUATION_TYPE *getWeights()
	{
		return AnyLoader::getData<Board::EVALUATION_TYPE>();
	}
public:
	DEF OthelloPlayer *getPlayer(int seed, bool negated)
	{
        return new WPCPlayer(getWeights(), negated, seed);
	}

	DEF int getNWeights()
	{
		return N_WEIGHTS;
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
	{
		memcpy(weights, getWeights(), sizeof(Board::EVALUATION_TYPE) * getNWeights());
	}

	DEF void setWeights(Board::EVALUATION_TYPE *weights)
	{
		memcpy(getWeights(), weights, sizeof(Board::EVALUATION_TYPE) * getNWeights());
	}

	static WPCLoader* load(const std::string &filename)
	{
		BinaryReader reader;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!reader.readFromFile(file))
		{
			printf("Nie moge odczytac pliku: %s\n", filename.c_str());
			return nullptr;
		}
		file.close();

		int type;
		reader.read(type);

		if (type != WPC_FORMAT)
			return nullptr;

		return load(reader);
	}

	static WPCLoader* load(BinaryReader &reader)
	{
		char *data = getData();
		Board::EVALUATION_TYPE *tmpData = getWeights(data);

		for(int i = 0; i < N_WEIGHTS; i++)
		{
			double value;
			reader.read(value);
			Board::EVALUATION_TYPE val = (Board::EVALUATION_TYPE)value;
			*(tmpData++) = val;
		}

		return new WPCLoader(data, getSize(), true);
	}
protected:
	DEF WPCLoader(char *data, unsigned size, bool ownData)
		: PlayerLoader(data, size, ownData) { }
private:
	DEF static char *getData()
	{
		int totalSize = getSize();
		char *data = new char[totalSize];
		{
			int *ptr = reinterpret_cast<int*>(data);
			ptr[0] = WPC_FORMAT;
		}
		return data;
	}

	DEF static unsigned getSize()
	{
		return sizeof(int) + N_WEIGHTS * sizeof(Board::EVALUATION_TYPE);
	}

	DEF static Board::EVALUATION_TYPE *getWeights(char *data)
	{
		int *intPtr = reinterpret_cast<int *>(data);
		return reinterpret_cast<Board::EVALUATION_TYPE *>(intPtr + 1);
	}
};

class MultiLoader : public PlayerLoader
{
	friend AnyLoader;
	friend PlayerLoader;
public:
	DEF OthelloPlayer *getPlayer(int seed, bool negated)
	{
	    char *data = getRawData();
	    int nPlayers = *getNPlayers(data);
        OthelloPlayer **players = new OthelloPlayer*[nPlayers];
        Board::EVALUATION_TYPE *nPawns = new Board::EVALUATION_TYPE[nPlayers];
		for(int i = 0; i < nPlayers; i++)
        {
            PlayerLoader *loader = getPlayerLoader(data, i, nPlayers);
            bool negated = getPlayerNegation(data, i, nPlayers);
            players[i] = loader->getPlayer(seed, negated);
            delete loader;
            nPawns[i] = (Board::EVALUATION_TYPE)getPlayerNPawns(data, i);
        }

        OthelloPlayer *player = ::getPlayer(nPlayers, players, nPawns, seed);

        delete[] nPawns;
        delete[] players;

        return player;
	}

	DEF int getNWeights()
	{
	    return *getNWeights(getRawData());
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
	{
	    int nPlayers = *getNPlayers(getRawData());
		for(int i = 0; i < nPlayers; i++)
            *(weights++) = (Board::EVALUATION_TYPE)getPlayerNPawns(getRawData(), i);
        for(int i = 0; i < nPlayers; i++)
        {
            PlayerLoader *loader = getPlayerLoader(getRawData(), i, nPlayers);
            loader->getWeights(weights);
            weights += loader->getNWeights();
            delete loader;
        }
	}

	DEF void setWeights(Board::EVALUATION_TYPE *weights)
	{
	    int nPlayers = *getNPlayers(getRawData());
	    auto pawns = getNPawns(getRawData());
		for(int i = 0; i < nPlayers; i++)
            pawns[i] = (float)*(weights++);
        for(int i = 0; i < nPlayers; i++)
        {
            PlayerLoader *loader = getPlayerLoader(getRawData(), i, nPlayers);
            loader->setWeights(weights);
            weights += loader->getNWeights();
            delete loader;
        }
	}

	static MultiLoader* load(const std::string &filename)
	{
		BinaryReader reader;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!reader.readFromFile(file))
		{
			printf("Nie moge odczytac pliku: %s\n", filename.c_str());
			return nullptr;
		}
		file.close();

		int type;
		reader.read(type);

		if (type != MULTI_PLAYER_FORMAT)
			return nullptr;

		return load(reader);
	}

	static MultiLoader* load(BinaryReader &reader)
	{
		/*
	    int nPlayers;
	    reader.read(nPlayers);
	    float *nPawns = new float[nPlayers];
	    bool *negated = new bool[nPlayers];
	    for(int i = 0; i < nPlayers; i++)
            reader.read(nPawns[i]);
        for(int i = 0; i < nPlayers; i++)
            reader.read(negated[i]);
        PlayerLoader **loaders = new PlayerLoader*[nPlayers];
        for(int i = 0; i < nPlayers; i++)
            loaders[i] = PlayerLoader::getLoader(reader);

		char *data = getData(nPlayers, loaders, nPawns, negated);
		int size = getSize(nPlayers, loaders);

        for(int i = 0; i < nPlayers; i++)
            delete loaders[i];
        delete[] loaders;
        delete[] negated;
        delete[] nPawns;*/

		auto d = reader.copyData();
		auto data = d.getData();
		auto size = d.getSize();

		return new MultiLoader(data, size, true);
	}

	static MultiLoader *get(int nPlayers, PlayerLoader **loaders, float *nPawns, bool *negated)
	{
	    char *data = getData(nPlayers, loaders, nPawns, negated);
	    int size = getSize(nPlayers, loaders);
	    return new MultiLoader(data, size, true);
	}
protected:
	DEF MultiLoader(char *data, unsigned size, bool ownData)
		: PlayerLoader(data, size, ownData) { }
private:
	DEF static char *getData(int nPlayers, PlayerLoader **loaders, float *nPawns, bool *negated)
	{
		int totalSize = getSize(nPlayers, loaders);
		char *data = new char[totalSize];
		char *tmpData = data;

		int nWeights = nPlayers;
		for(int i = 0; i < nPlayers; i++)
            nWeights += loaders[i]->getNWeights();
		{
			int *ptr = reinterpret_cast<int*>(tmpData);
			ptr[0] = MULTI_PLAYER_FORMAT;
			ptr[1] = nPlayers;
			ptr[2] = nWeights;
			tmpData = reinterpret_cast<char*>(ptr + 3);
		}
		for(int i = 0; i < nPlayers; i++)
        {
            float *ptr = reinterpret_cast<float*>(tmpData);
			ptr[0] = nPawns[i];
			tmpData = reinterpret_cast<char*>(ptr + 1);
        }
		for(int i = 0; i < nPlayers; i++)
        {
            bool *ptr = reinterpret_cast<bool*>(tmpData);
			ptr[0] = negated[i];
			tmpData = reinterpret_cast<char*>(ptr + 1);
        }
		for(int i = 0; i < nPlayers; i++)
        {
            int *ptr = reinterpret_cast<int*>(tmpData);
            int size = loaders[i]->getRawDataSize();
			ptr[0] = size;
			tmpData = reinterpret_cast<char*>(ptr + 1);
        }
		for(int i = 0; i < nPlayers; i++)
        {
            int size = loaders[i]->getRawDataSize();
            memcpy(tmpData, loaders[i]->getRawData(), size);
            tmpData += size;
        }
		return data;
	}

	DEF static unsigned getSize(int nPlayers, PlayerLoader **loaders)
	{
	    // type + nPlayers
		int result = sizeof(int) * 2;
		// nWeights
        result += sizeof(int);
		// nPawns
		result += sizeof(int) * nPlayers;
		// negation
		result += sizeof(bool) * nPlayers;
		// size
		result += sizeof(int) * nPlayers;

        for(int i = 0; i < nPlayers; i++)
            result += loaders[i]->getRawDataSize();

		return result;
	}

	DEF static int *getNPlayers(char *data)
	{
		int *intPtr = reinterpret_cast<int *>(data);
		return intPtr + 1;
	}

	DEF static int *getNWeights(char *data)
	{
		int *intPtr = reinterpret_cast<int *>(data);
		return intPtr + 2;
	}

	DEF static float *getNPawns(char *data)
	{
	    int *intPtr = reinterpret_cast<int *>(data);
	    return reinterpret_cast<float *>(intPtr + 3);
	}

	DEF static float getPlayerNPawns(char *data, int playerNumber)
	{
	    return getNPawns(data)[playerNumber];
	}

	DEF static bool *getNegation(char *data)
	{
	    int nPlayers = *getNPlayers(data);
	    return getNegation(data, nPlayers);
	}

	DEF static bool *getNegation(char *data, int nPlayers)
	{
	    return reinterpret_cast<bool*>(getNPawns(data) + nPlayers);
	}

	DEF static bool getPlayerNegation(char *data, int playerNumber)
	{
	    int nPlayers = *getNPlayers(data);
	    return getPlayerNegation(data, playerNumber, nPlayers);
	}

	DEF static bool getPlayerNegation(char *data, int playerNumber, int nPlayers)
	{
	    return getNegation(data, nPlayers)[playerNumber];
	}

	DEF static int *getSize(char *data)
	{
	    int nPlayers = *getNPlayers(data);
	    return getSize(data, nPlayers);
	}

	DEF static int *getSize(char *data, int nPlayers)
	{
	    return reinterpret_cast<int*>(getNegation(data) + nPlayers);
	}

	DEF static int getPlayerSize(char *data, int playerNumber)
	{
	    int nPlayers = *getNPlayers(data);
	    return getPlayerSize(data, playerNumber, nPlayers);
	}

	DEF static int getPlayerSize(char *data, int playerNumber, int nPlayers)
	{
	    return getSize(data, nPlayers)[playerNumber];
	}

	DEF static PlayerLoader *getPlayerLoader(char *data, int playerNumber)
	{
	    int nPlayers = *getNPlayers(data);
	    return getPlayerLoader(data, playerNumber, nPlayers);
	}

	DEF static PlayerLoader *getPlayerLoader(char *data, int playerNumber, int nPlayers)
	{
	    int *sizes = getSize(data, nPlayers);
	    int offset = 0;
	    for(int i = 0; i < playerNumber; i++)
            offset += sizes[i];
        char *loaderData = reinterpret_cast<char *>(sizes + nPlayers) + offset;
        return PlayerLoader::getLoader(loaderData, sizes[playerNumber], false);
	}
};

class NTupleLoader : public PlayerLoader
{
	friend AnyLoader;
	friend PlayerLoader;

	DEF Board::EVALUATION_TYPE *getWeights()
	{
		return reinterpret_cast<Board::EVALUATION_TYPE*>(getFields() + getNFields());
	}
public:
	DEF OthelloPlayer *getPlayer(int seed, bool negated)
	{
		return getNTuplePlayer(seed, negated);
	}

	DEF int getNFields()
	{
		return AnyLoader::getData<int>()[0];
	}

	DEF int getNWeights()
	{
		return AnyLoader::getData<int>()[1];
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
	{
		memcpy(weights, getWeights(), sizeof(Board::EVALUATION_TYPE) * getNWeights());
	}

	DEF void setWeights(Board::EVALUATION_TYPE *weights)
	{
		memcpy(getWeights(), weights, sizeof(Board::EVALUATION_TYPE) * getNWeights());
	}

	DEF int getNTuples()
	{
		return AnyLoader::getData<int>()[2];
	}

	DEF int getMaxTuplePerPos()
	{
		return AnyLoader::getData<int>()[3];
	}

	DEF Board::INDEX_TYPE *getFields()
	{
		return reinterpret_cast<Board::INDEX_TYPE*>(AnyLoader::getData<int>() + 4);
	}

	DEF int *getTuples()
	{
		return reinterpret_cast<int*>(getWeights() + getNWeights());
	}

	static NTupleLoader* load(const std::string &filename)
	{
		BinaryReader reader;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!reader.readFromFile(file))
		{
			printf("Nie moge odczytac pliku: %s\n", filename.c_str());
			return nullptr;
		}
		file.close();

		int type;
		reader.read(type);

		if (type != N_TUPLE_FORMAT)
			return nullptr;

		return load(reader);
	}

	static NTupleLoader* load(BinaryReader &reader)
	{
		int nFields, nWeights, nTuples;
		reader.read(nFields);
		reader.read(nWeights);
		reader.read(nTuples);
		char *data = getData(nFields, nWeights, nTuples);

		Board::INDEX_TYPE *fieldsData = getFields(data);
		Board::INDEX_TYPE *fields = fieldsData;
		for(int i = 0; i < nFields; i++)
		{
			int value;
			reader.read(value);
			Board::INDEX_TYPE val = (Board::INDEX_TYPE)value;
			*(fieldsData++) = val;
		}

		Board::EVALUATION_TYPE *weightsData = reinterpret_cast<Board::EVALUATION_TYPE*>(fieldsData);
		for(int i = 0; i < nWeights; i++)
		{
			double value;
			reader.read(value);
			Board::EVALUATION_TYPE val = (Board::EVALUATION_TYPE)value;
			*(weightsData++) = val;
		}

		int *intData = reinterpret_cast<int*>(weightsData);
		int *tuples = intData;
		for(int i = 0; i < nTuples * 3; i++)
		{
			int value;
			reader.read(value);
			*(intData++) = value;
		}

		int fieldCounts[100] = { 0 };
		for (int i = 0; i < nTuples; i++)
		{
			int index = tuples[i * 3 + 1];
			for (int j = 0; j < tuples[i * 3]; j++)
				fieldCounts[fields[index + j]]++;
		}

		int maxTuplePerPos = 0;
		for (int i = 0; i < 100; i++)
		{
			if (maxTuplePerPos < fieldCounts[i])
				maxTuplePerPos = fieldCounts[i];
		}

		setMaxTuplePerPos(data, maxTuplePerPos);

		return new NTupleLoader(data, getSize(nFields, nWeights, nTuples), true);
	}
protected:
	DEF NTupleLoader(char *data, unsigned size, bool ownData)
		: PlayerLoader(data, size, ownData) { }
private:
	DEF static char *getData(int nFields, int nWeights, int nTuples)
	{
		int totalSize = getSize(nFields, nWeights, nTuples);
		char *data = new char[totalSize];
		{
			int *ptr = reinterpret_cast<int*>(data);
			*(ptr++) = N_TUPLE_FORMAT;
			*(ptr++) = nFields;
			*(ptr++) = nWeights;
			*(ptr++) = nTuples;
		}
		return data;
	}

	DEF static void setMaxTuplePerPos(char *data, int value)
	{
		int *ptr = reinterpret_cast<int*>(data);
		ptr[4] = value;
	}

	DEF static unsigned getSize(int nFields, int nWeights, int nTuples)
	{
		return 5 * sizeof(int)
			+ nFields * sizeof(Board::INDEX_TYPE)
			+ nWeights * sizeof(Board::EVALUATION_TYPE)
			+ 3 * nTuples * sizeof(int);
	}

	DEF static Board::INDEX_TYPE *getFields(char *data)
	{
		int *intPtr = reinterpret_cast<int *>(data);
		return reinterpret_cast<Board::INDEX_TYPE *>(intPtr + 5);
	}

	DEF OthelloPlayer *getNTuplePlayer(int seed, bool negated)
	{
		OthelloPlayer *player = nullptr;

		player = check<632, 6561, 120, 18>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<624, 1701, 156, 19>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<576, 8748, 96, 30>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<576, 8748, 96, 22>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<540, 648, 180, 13>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<520, 3402, 104, 16>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<488, 4698, 96, 20>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<480, 288, 240, 11>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<464, 3240, 96, 36>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<64, 192, 64, 1>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<64, 30, 64, 1>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<8, 18, 4, 3>(seed, negated);
		if (player != nullptr)
			return player;

		player = check<6, 9, 3, 1>(seed, negated);
		if (player != nullptr)
			return player;

		return nullptr;
	}

	template <int N_FIELDS, int N_WEIGHTS, int N_TUPLES, int TUPLES_PER_POS>
	DEF OthelloPlayer *check(int seed, bool negated)
	{
		if (getNFields() == N_FIELDS &&
			getNWeights() == N_WEIGHTS &&
			getNTuples() == N_TUPLES &&
			getMaxTuplePerPos() == TUPLES_PER_POS)
		{
			OthelloPlayer *player = new NTuplePlayer<N_FIELDS, N_WEIGHTS, N_TUPLES, TUPLES_PER_POS>(seed, negated, getFields(), getWeights(), getTuples());
			return player;
		}

		return nullptr;
	}
};

DEF AnyLoader *AnyLoader::getLoader(char *data, unsigned dataSize, bool own)
{
    int type = getType(data);
    if (isBoardFormat(type))
        return new BoardLoader(data, dataSize, own);
    if (isPlayerFormat(type))
        return PlayerLoader::getLoader(data, dataSize, own);

    return nullptr;
}

AnyLoader *AnyLoader::getLoader(std::string filename)
{
	BinaryReader reader;

	std::ifstream file;
	file.open(filename.c_str(), std::ios::in | std::ios::binary);
	if (!reader.readFromFile(file))
	{
		printf("Nie moge odczytac pliku: %s\n", filename.c_str());
		return nullptr;
	}
	file.close();

	return getLoader(reader);
}

AnyLoader *AnyLoader::getLoader(BinaryReader &reader)
{
	int type;
	reader.read(type);

	switch(type)
	{
	case BOARD_FORMAT:
		return BoardLoader::load(reader);
	case WPC_FORMAT:
		return WPCLoader::load(reader);
	case N_TUPLE_FORMAT:
		return NTupleLoader::load(reader);
	default:
		return nullptr;
	}
}

DEF PlayerLoader *PlayerLoader::getLoader(char *data, unsigned dataSize, bool own)
{
    int type = getType(data);
    switch(type)
	{
        case WPC_FORMAT:
            return new WPCLoader(data, dataSize, own);
        case N_TUPLE_FORMAT:
            return new NTupleLoader(data, dataSize, own);
        case MULTI_PLAYER_FORMAT:
            return new MultiLoader(data, dataSize, own);
        default:
            return nullptr;
	}
}

PlayerLoader *PlayerLoader::getLoader(std::string filename)
{
    BinaryReader reader;

	std::ifstream file;
	file.open(filename.c_str(), std::ios::in | std::ios::binary);
	if (!reader.readFromFile(file))
	{
		printf("Nie moge odczytac pliku: %s\n", filename.c_str());
		return nullptr;
	}
	file.close();

	return getLoader(reader);
}

PlayerLoader *PlayerLoader::getLoader(BinaryReader &reader)
{
	int type;
	reader.read(type);

	switch(type)
	{
        case WPC_FORMAT:
            return WPCLoader::load(reader);
        case N_TUPLE_FORMAT:
            return NTupleLoader::load(reader);
        case MULTI_PLAYER_FORMAT:
            return MultiLoader::load(reader);
        default:
            return nullptr;
	}
}

AnyLoader *AnyLoader::getLoaderBinary(std::string filename)
{
    Data *d = BinaryReader::getData(filename);
    auto data = d->getData();
    auto dataSize = d->getSize();
    delete d;
    int type = reinterpret_cast<int*>(data)[0];

	switch(type)
	{
        case BOARD_FORMAT:
            return new BoardLoader(data, dataSize, true);
        case WPC_FORMAT:
            return new WPCLoader(data, dataSize, true);
        case N_TUPLE_FORMAT:
            return new NTupleLoader(data, dataSize, true);
        case MULTI_PLAYER_FORMAT:
            return new MultiLoader(data, dataSize, true);
        default:
            return nullptr;
	}
}

PlayerLoader *PlayerLoader::getLoaderBinary(std::string filename)
{
    Data *d = BinaryReader::getData(filename);
    auto data = d->getData();
    auto dataSize = d->getSize();
    delete d;
    int type = reinterpret_cast<int*>(data)[0];

	switch(type)
	{
        case WPC_FORMAT:
            return new WPCLoader(data, dataSize, true);
        case N_TUPLE_FORMAT:
            return new NTupleLoader(data, dataSize, true);
        case MULTI_PLAYER_FORMAT:
            return new MultiLoader(data, dataSize, true);
        default:
            return nullptr;
	}
}

#endif // ANY_LOADER_H
