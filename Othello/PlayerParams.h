#ifndef PLAYER_PARAMS_H
#define PLAYER_PARAMS_H

#include "Header.h"
#include "Random.h"
#include "Board.h"

class PlayerParams
{
public:
	DEF PlayerParams(int seed)
		: rand(seed) { }

	DEF virtual ~PlayerParams()
	{
	}

	Random<int> rand;
	Vector<Board::INDEX_TYPE, Board::SIZE> bestMoves;
};

template<int N_TUPLES>
class NTuplePlayerParams : public PlayerParams
{
public:
	DEF NTuplePlayerParams(int seed) :
		PlayerParams(seed) { }

	SparseSet<unsigned short, N_TUPLES, N_TUPLES> indexes;
};

template<int N_PLAYERS>
class MultiPlayerParams : public PlayerParams
{
public:
	DEF MultiPlayerParams(int seed) :
		PlayerParams(seed) { }

	DEF ~MultiPlayerParams()
	{
		for(int i = 0; i < params.size(); i++)
			delete params[i];
	}

	Vector<PlayerParams *, N_PLAYERS> params;
};

class GameData
{
public:
    DEF GameData() { }

	Board tmpBoard;
	SparseSet<Board::BOARD_ELEMENT_TYPE, Board::SIZE, Board::MAX_CH_POS> positions;
};

#endif // PLAYER_PARAMS_H
