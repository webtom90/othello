#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <float.h>
#include "Header.h"
#include "Board.h"
#include "NTuples.h"
#include "Random.h"
#include "Vector.h"

class OthelloPlayer;
class CpuPlayer1;
template <int N> class TuplePlayer;

// Wskazuje ruch w grze.
class OthelloPlayer
{
public:
	typedef OthelloPlayer BASE_PLAYER_TYPE;
	typedef TuplePlayer<2> CURRENT_PLAYER_TYPE;

	// Zwraca najlepszy swoim zdaniem ruch dla zadanego stanu.
	DEF virtual Board::INDEX_TYPE getMove(Board &board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> &validMoves) = 0;
};

class CpuPlayer : public OthelloPlayer
{
#define EPS_VALUE 0.00001
	Random<int> random;
public:
	DEF CpuPlayer(int seed) : random(seed) { }
	DEF Board::INDEX_TYPE getMove(Board &board, Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> &validMoves)
	{
		Vector<Board::INDEX_TYPE, Board::SIZE> bestMoves;
		Board::EVALUATION_TYPE bestEvaluation = -FLT_MAX;
		for (int i = 0; i < validMoves.size(); i++)
		{
			Board::EVALUATION_TYPE value = evaluateMove(board, validMoves[i]);
			if (bestEvaluation < value)
			{
				bestEvaluation = value;
				bestMoves.clear();
				bestMoves.add(validMoves[i]);
			}
			else if (bestEvaluation - EPS_VALUE < value)
			{
				bestMoves.add(validMoves[i]);
			}
		}
		if (bestMoves.size() > 0)
		{
			int chosen = random.getValue(bestMoves.size());
			return bestMoves[chosen];
			
		}
		return -1;
	}
private:
	// Ocena wskazanego ruchu.
	DEF virtual Board::EVALUATION_TYPE evaluateMove(Board &board, Board::INDEX_TYPE move) = 0;
};

class CpuPlayer1 : public CpuPlayer
{
public:
	DEF CpuPlayer1(int seed) : CpuPlayer(seed) { }
private:
	// Ocena wskazanego ruchu.
	DEF Board::EVALUATION_TYPE evaluateMove(Board &board, Board::INDEX_TYPE move)
	{
		Board tmpBoard(board);
		Vector<Board::BOARD_ELEMENT_TYPE, Board::MAX_CH_POS> p;
		tmpBoard.simulateMove(p, move);
		tmpBoard.makeMove(move, p);
		int whites = 0;
		int blacks = 0;
		for(int y = 1; y < tmpBoard.WIDTH - 1; y++)
		{
			for(int x = 1; x < tmpBoard.WIDTH - 1; x++)
			{
				Board::BOARD_ELEMENT_TYPE value = tmpBoard.getValue(x, y);
				if (value == tmpBoard.WHITE)
					whites++;
				if (value == tmpBoard.BLACK)
					blacks++;
			}
		}
		
		return (Board::EVALUATION_TYPE)whites / blacks;
	}
};

class CpuPlayer2 : public CpuPlayer
{
public:
	DEF CpuPlayer2(int seed) : CpuPlayer(seed)
	{
		c = 0;
	}
	int c;
private:
	// Ocena wskazanego ruchu.
	DEF Board::EVALUATION_TYPE evaluateMove(Board &board, Board::INDEX_TYPE move)
	{
		c = ++c % 200;
		return c + move;
	}
};

template <int N>

class TuplePlayer : public CpuPlayer
{
public:
	DEF TuplePlayer(int seed) : CpuPlayer(seed) { }
private:
	// Ocena wskazanego ruchu.
	DEF Board::EVALUATION_TYPE evaluateMove(Board &board, Board::INDEX_TYPE move)
	{
		return nTuples.getValue(board, move);
	}

	NTuples<N> nTuples;
};

#endif //OTHELLO_PLAYER_H

