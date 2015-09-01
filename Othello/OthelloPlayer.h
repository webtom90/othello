#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <limits.h>
#include <string>
#include <cmath>
#include <algorithm>
#include "Header.h"
#include "Board.h"
#include "Vector.h"
#include "TupleLoader.h"
#include "NTuples.h"

class OthelloPlayer;
template <bool negated>
class CpuPlayer1;
template <int N_PLAYERS>
class MultiPlayer;

// Wskazuje ruch w grze.
class OthelloPlayer
{
public:
    DEF OthelloPlayer()
    {
        randomMoveFreq = 0;
    }

	DEF virtual ~OthelloPlayer() { }

	typedef OthelloPlayer BASE_PLAYER_TYPE;

	// Zwraca najlepszy swoim zdaniem ruch dla zadanego stanu.
	DEF virtual Board::INDEX_TYPE getMove(Board *board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, Board::BOARD_ELEMENT_TYPE playerd, bool negated, PlayerParams *p, GameData *data) = 0;

	DEF virtual bool isNegated(Board *board)
	{
		return false;
	}

	DEF virtual int getNWeights() = 0;
	DEF virtual void getWeights(Board::EVALUATION_TYPE *weights) = 0;
	DEF virtual void setWeights(const Board::EVALUATION_TYPE *weights) = 0;

	static OthelloPlayer *getNewPlayer(const std::string &filename, int seed, bool negated);
	DEF static OthelloPlayer *getPlayer(UniversalLoader *loader, int seed, bool negated);
	static OthelloPlayer *getPlayer(const std::string &filename, int seed, bool negated);

	DEF float getRandomMoveFreq()
	{
		return randomMoveFreq;
	}

	DEF void setRandomMoveFreq(float value)
	{
		randomMoveFreq = value;
		_setRandomMoveFreq(value);
	}

	DEF virtual PlayerParams *getPlayerParams(int seed) = 0;
private:
	float randomMoveFreq;
protected:
    DEF void _setRandomMoveFreq(float value) { }
};

class ConsoleOthelloPlayer : public OthelloPlayer
{
	DEF Board::INDEX_TYPE getMove(Board *board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, Board::BOARD_ELEMENT_TYPE player, bool negated, PlayerParams *p, GameData *data)
	{
		#if WIN32
		system("cls");
		#endif
		board->print();
		if (validMoves->size() == 0)
			return -1;

		Board::INDEX_TYPE field;
		while(true)
		{
		    int value;
			if (!scanf("%d", &value))
				return validMoves[0][0];
			field = static_cast<Board::INDEX_TYPE>(value);

			if (validMoves->containsBegin(field))
				break;
		}
		return field;
	}

	DEF int getNWeights()
	{
		return 0;
	}

    DEF void getWeights(Board::EVALUATION_TYPE *weights) { }

    DEF void setWeights(const Board::EVALUATION_TYPE *weights) { }

	DEF PlayerParams *getPlayerParams(int seed)
	{
        return nullptr;
	}
};

class CpuPlayer : public OthelloPlayer
{
    template <int N_PLAYERS> friend class MultiPlayer;
#define EPS_VALUE 0.00001
public:
	DEF CpuPlayer(int seed, bool negated)
	{
	    this->negated = negated;
	}

	DEF Board::INDEX_TYPE getMove(Board *board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, Board::BOARD_ELEMENT_TYPE player, bool negated, PlayerParams *p,
                               #ifndef ON_STACK
                               GameData *data
                               #endif //ON_STACK
                               )
	{
#if 0
		auto val = evaluateMove(board, validMoves[0][0], player, p, data);
		Board::INDEX_TYPE result = validMoves[0][0];
		return result;
#else
		//bool negate = isNegated() && player == Board::WHITE;
		p->bestMoves.clear();
		Board::EVALUATION_TYPE bestEvaluation = Board::WORSE_EVAL;

		for (int i = 0; i < validMoves->size(); i++)
		{
			Board::EVALUATION_TYPE value;
			if (negated)
				value = -evaluateMove(board, validMoves[0][i], player, p, data);
			else
				value = evaluateMove(board, validMoves[0][i], player, p, data);

			if (value == bestEvaluation || std::abs(value - bestEvaluation) < EPS_VALUE)
			{
				p->bestMoves.add(validMoves[0][i]);
			}
			else if (bestEvaluation < value || i == 0)
			{
				bestEvaluation = value;
				p->bestMoves.clear();
				p->bestMoves.add(validMoves[0][i]);
			}
		}

		switch (p->bestMoves.size())
		{
		case 0:
		    printf("Invalid game state - player has no moves.\n");
			exit(0);
			return -1;
		case 1:
			return p->bestMoves[0];
		default:
#ifdef MIN_MOVE
			Board::INDEX_TYPE min = 101;
			for (int i = 0; i < bestMoves.size(); i++)
			if (min > p->bestMoves[i])
				min = p->bestMoves[i];
			return min;
#else
			int chosen = p->rand.getValue(p->bestMoves.size());
			return p->bestMoves[chosen];
#endif
		}
#endif
	}

	DEF virtual bool isNegated(Board *board)
	{
		return negated;
	}
protected:
	// Ocena wskazanego ruchu.
	DEF virtual Board::EVALUATION_TYPE evaluateMove(Board *board, Board::INDEX_TYPE move, Board::BOARD_ELEMENT_TYPE player, PlayerParams *p, GameData *data) = 0;
private:
    bool negated;
};

class WPCPlayer : public CpuPlayer
{
public:
	DEF WPCPlayer(Board::EVALUATION_TYPE *weights, bool negated, int seed)
		: CpuPlayer(seed, negated)
	{
			memcpy(this->weights, weights, sizeof(this->weights));
	}
private:
	Board::EVALUATION_TYPE weights[64];

protected:
	DEF Board::EVALUATION_TYPE evaluateMove(Board *board, Board::INDEX_TYPE move, Board::BOARD_ELEMENT_TYPE player, PlayerParams *p, GameData *data)
	{
		data->tmpBoard.copy(board);
		data->tmpBoard.simulateMove(&(data->positions), move, player);

		Board::EVALUATION_TYPE result = 0;
		for (int i = 0; i < data->positions.size(); i++)
		{
			result -= getValue(data->positions[i], data->tmpBoard);
		}
		data->tmpBoard.performMove(&(data->positions), player);
		for (int i = 0; i < data->positions.size(); i++)
		{
			result += getValue(data->positions[i], data->tmpBoard);
		}

		return result;
	}

private:
	DEF Board::EVALUATION_TYPE getValue(Board::INDEX_TYPE index, Board &board)
	{
		return weights[getWeightIndex(index)] * board.getValue(index);
	}

	DEF Board::INDEX_TYPE getWeightIndex(Board::INDEX_TYPE index)
	{
		Board::INDEX_TYPE x = index % Board::WIDTH - 1;
		Board::INDEX_TYPE y = index / Board::WIDTH - 1;

		return x + 8 * y;
	}

	DEF int getNWeights()
	{
		return 64;
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
    {
        memcpy(weights, this->weights, sizeof(Board::EVALUATION_TYPE) * 64);
    }

    DEF void setWeights(const Board::EVALUATION_TYPE *weights)
    {
        memcpy(this->weights, weights, sizeof(Board::EVALUATION_TYPE) * 64);
    }

	DEF PlayerParams *getPlayerParams(int seed)
	{
		return new PlayerParams(seed);
	}
};

template <int N_FIELDS, int N_WEIGHTS, int N_TUPLES, int TUPLES_PER_FIELD>
class NTuplePlayer : public CpuPlayer
{
public:
	DEF NTuplePlayer(int seed, bool negated, UniversalLoader *loader)
		: CpuPlayer(seed, negated), nTuples(loader->getFields(), loader->getWeights(), loader->getTuples())
	{
	}

	DEF NTuplePlayer(int seed, bool negated, Board::INDEX_TYPE *fields, Board::EVALUATION_TYPE *weights, int *tuples)
		: CpuPlayer(seed, negated), nTuples(fields, weights, tuples)
	{
	}

	DEF int getNWeights()
	{
		return N_WEIGHTS;
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
    {
        memcpy(weights, nTuples.getWeights(), sizeof(Board::EVALUATION_TYPE) * N_WEIGHTS);
    }

    DEF void setWeights(const Board::EVALUATION_TYPE *weights)
    {
        memcpy(nTuples.getWeights(), weights, sizeof(Board::EVALUATION_TYPE) * N_WEIGHTS);
    }

	DEF PlayerParams *getPlayerParams(int seed)
	{
		return new NTuplePlayerParams<N_TUPLES>(seed);
	}
protected:
	// Ocena wskazanego ruchu.
	DEF Board::EVALUATION_TYPE evaluateMove(Board *board, Board::INDEX_TYPE move, Board::BOARD_ELEMENT_TYPE player, PlayerParams *p, GameData *data)
	{
		NTuplePlayerParams<N_TUPLES> *par = reinterpret_cast<NTuplePlayerParams<N_TUPLES> *>(p);
		return nTuples.getValue(board, move, player, par, data);
	}
private:
	NTuples<N_FIELDS, N_WEIGHTS, N_TUPLES, TUPLES_PER_FIELD> nTuples;
};

template <int N_PLAYERS>
class MultiPlayer : public OthelloPlayer
{
public:
	DEF MultiPlayer(OthelloPlayer **players, Board::EVALUATION_TYPE *nPawns, int seed)
		: OthelloPlayer()
	{
	    for(int i = 0; i < N_PLAYERS; i++)
            this->players.add(players[i]);
        setNPawns(nPawns);
	}
protected:
    DEF void _setRandomMoveFreq(float value)
    {
        for(int i = 0; i < N_PLAYERS; i++)
            players[i]->setRandomMoveFreq(value);
    }
private:
    Vector<OthelloPlayer*, N_PLAYERS> players;
    Vector<Board::EVALUATION_TYPE, N_PLAYERS> nPawns;
    Vector<int, N_PLAYERS> indexes;

    DEF int getIndex(int nPawns, int min, int max)
    {
        if (max < min)
            return min - 1;

        int mid = min + (max - min) / 2;
		int midIndex = indexes[mid];
		Board::EVALUATION_TYPE value = this->nPawns[midIndex];
        if (value < nPawns)
            return getIndex(nPawns, mid + 1, max);
        else if (value > nPawns)
            return getIndex(nPawns, min, mid - 1);
        else
            return mid - 1;
    }

    DEF int getPlayerIndex(int nPawns)
    {
        //return 0;
        return getIndex(nPawns, 0, N_PLAYERS - 2) + 1;
    }
public:
    DEF Board::INDEX_TYPE getMove(Board *board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, Board::BOARD_ELEMENT_TYPE playerd, bool negated, PlayerParams *p, GameData *data)
    {
        int index = getPlayerIndex(board->getNPawns());
		MultiPlayerParams<N_PLAYERS> *par = reinterpret_cast<MultiPlayerParams<N_PLAYERS> *>(p);
		return players[index]->getMove(board, validMoves, playerd, negated, par->params[index], data);
    }

	DEF int getNWeights()
	{
	    int result = N_PLAYERS;
	    for(int i = 0; i < N_PLAYERS; i++)
            result += players[i]->getNWeights();
		return result;
	}

	DEF void getWeights(Board::EVALUATION_TYPE *weights)
    {
        for(int i = 0; i < N_PLAYERS; i++)
            weights[i] = (Board::EVALUATION_TYPE)(nPawns[i] / 100);
        int offset = N_PLAYERS;
        for(int i = 0; i < N_PLAYERS; i++)
        {
            OthelloPlayer *player = players[i];
            player->getWeights(weights + offset);
            offset += player->getNWeights();
        }
    }

    DEF void setWeights(const Board::EVALUATION_TYPE *weights)
    {
        setNPawns(weights);
        int offset = N_PLAYERS;
        for(int i = 0; i < N_PLAYERS; i++)
        {
            OthelloPlayer *player = players[i];
            player->setWeights(weights + offset);
            offset += player->getNWeights();
        }
    }

	DEF PlayerParams *getPlayerParams(int seed)
	{
		MultiPlayerParams<N_PLAYERS> *result = new MultiPlayerParams<N_PLAYERS>(seed);
		for(int i = 0; i < N_PLAYERS; i++)
            result->params.add(players[i]->getPlayerParams(seed));
		return result;
	}

	DEF bool isNegated(Board *board)
	{
	    int index = getPlayerIndex(board->getNPawns());
		return players[index]->isNegated(board);
	}
private:
    DEF void sortNPawns()
    {
        for(int i = 1; i < N_PLAYERS; i++)
        {
            int key = indexes[i];
            Board::EVALUATION_TYPE value = nPawns[key];
            int j = i - 1;
            while (j >= 0 && nPawns[indexes[j]] > value)
            {
                indexes[j+1] = indexes[j];
                j--;
            }
            indexes[j+1] = key;
        }
    }

    DEF void setNPawns(const Board::EVALUATION_TYPE *weights)
    {
        const int MAX_N_PAWNS = (64 - 11) / N_PLAYERS;
		this->nPawns.clear();
		this->indexes.clear();
        for(int i = 0; i < N_PLAYERS; i++)
        {
            int mid = (64 - 11) * (i + 1) / N_PLAYERS + 11;
            Board::EVALUATION_TYPE val = weights[i];
            val = std::max((Board::EVALUATION_TYPE)-MAX_N_PAWNS, val);
            val = std::min((Board::EVALUATION_TYPE)MAX_N_PAWNS, val);
			//this->nPawns.add(weights[i] * 100);
			//this->nPawns.add((64.0f - 11) * (i + 1) / N_PLAYERS + 11);
			this->nPawns.add(mid + val);
			this->indexes.add(i);
        }
        sortNPawns();
    }
};

template <int N_FIELDS, int N_WEIGHTS, int N_TUPLES, int TUPLES_PER_POS>
DEF OthelloPlayer *check(int seed, bool negated, UniversalLoader *loader)
{
	if (loader->getNFields() == N_FIELDS &&
		loader->getNWeights() == N_WEIGHTS &&
		loader->getNTuples() == N_TUPLES &&
		loader->getMaxTuplePerPos() == TUPLES_PER_POS)
	{
		OthelloPlayer *player = new NTuplePlayer<N_FIELDS, N_WEIGHTS, N_TUPLES, TUPLES_PER_POS>(seed, negated, loader);
		return player;
	}

	return nullptr;
}

DEF OthelloPlayer *_getNTuplePlayer(int seed, bool negated, UniversalLoader *loader)
{
	OthelloPlayer *player = nullptr;

	switch (loader->getType())
	{
		case UniversalLoader::N_TUPLE_FORMAT:
		{
			player = check<632, 6561, 120, 18>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<624, 1701, 156, 19>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<576, 8748, 96, 30>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<576, 8748, 96, 22>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<540, 648, 180, 13>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<520, 3402, 104, 16>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<488, 4698, 96, 20>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<480, 288, 240, 11>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<464, 3240, 96, 36>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<64, 192, 64, 1>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<64, 30, 64, 1>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<8, 18, 4, 3>(seed, negated, loader);
			if (player != nullptr)
				return player;

			player = check<6, 9, 3, 1>(seed, negated, loader);
			if (player != nullptr)
				return player;

			return nullptr;
		}
		case UniversalLoader::WPC_FORMAT:
		{
			if (loader->getNWeights() == 64)
			{
				player = new WPCPlayer(loader->getWeights(), negated, seed);
				return player;
			}

			return nullptr;
		}
	}
	return nullptr;
}

DEF OthelloPlayer *OthelloPlayer::getPlayer(UniversalLoader *loader, int seed, bool negated)
{
	OthelloPlayer *player = nullptr;

	player = _getNTuplePlayer(seed, negated, loader);

    if (player == nullptr)
    {
        printf("Unknown player: %d %d %d %d\n", loader->getNFields(), loader->getNWeights(), loader->getNTuples(), loader->getMaxTuplePerPos());
    }

	return player;
}

OthelloPlayer *OthelloPlayer::getPlayer(const std::string &filename, int seed, bool negated)
{
	OthelloPlayer *player = nullptr;
	UniversalLoader *loader = new UniversalLoader();
	if (!loader->load(filename))
	{
		delete loader;
		return nullptr;
	}

    player = getPlayer(loader, seed, negated);

	delete loader;

	return player;
}

DEF OthelloPlayer *getPlayer(int nPlayers, OthelloPlayer **players, Board::EVALUATION_TYPE *nPawns, int seed)
{
	switch(nPlayers)
	{
	case 2:
		return new MultiPlayer<2>(players, nPawns, seed);
	case 3:
		return new MultiPlayer<3>(players, nPawns, seed);
	case 4:
		return new MultiPlayer<4>(players, nPawns, seed);
	case 5:
		return new MultiPlayer<5>(players, nPawns, seed);
	case 6:
		return new MultiPlayer<6>(players, nPawns, seed);
	case 7:
		return new MultiPlayer<7>(players, nPawns, seed);
	case 8:
		return new MultiPlayer<8>(players, nPawns, seed);
	default:
		return nullptr;
	}
}

#endif //OTHELLO_PLAYER_H

