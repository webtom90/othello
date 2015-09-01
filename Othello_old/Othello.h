#ifndef OTHELLO_H
#define OTHELLO_H

#include "Header.h"
#include "OthelloPlayer.h"
#include "Board.h"
#include "Vector.h"

// Przeprowadza rozgrywkê.
class Othello
{
public:
	// Rozgrywa parê gier.
	DEF static Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> playDouble(OthelloPlayer::BASE_PLAYER_TYPE *p1, OthelloPlayer::BASE_PLAYER_TYPE *p2)
	{
		#if OUTPUT_MODE
		srand(0);
		#endif
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result1 = play(p1, p2);
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result2 = play(p2, p1);
		return aggregate(result1, result2);
	}

	// Rozgrywa pojedyncz¹ grê.
	DEF static Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> play(OthelloPlayer::BASE_PLAYER_TYPE *p1, OthelloPlayer::BASE_PLAYER_TYPE *p2)
	{
		Board board;
		Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> validMoves;
		SparseSet<Board::BOARD_ELEMENT_TYPE, 100, Board::SIZE> possibleMoves;
		board.possibleMoves(possibleMoves);
		bool aMoveWasPossible;
		OthelloPlayer::BASE_PLAYER_TYPE* players[] = { p1, p2 };
		do
		{
			aMoveWasPossible = false;
			for (int p = 0; p < 2; p++)
			{
				OthelloPlayer::BASE_PLAYER_TYPE *player = players[p];
				board.validMoves(validMoves, possibleMoves);
				if (validMoves.size() == 0)
				{
					continue;
				}
				
				int move = player->getMove(board, validMoves);
				/*int move = 0;*/
				board.makeMove(move);
				board.updatePossibleMoves(possibleMoves, move);
				aMoveWasPossible = true;
			}
		}
		while (aMoveWasPossible);

		Tuple<int, int> boardResult = board.result();
		return Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE>(boardResult.item1, boardResult.item2);
	}

	template <typename T1, typename T2>
	DEF static Tuple<T1, T2> aggregate(const Tuple<T1, T2> &t1, Tuple<T1, T2> &t2)
	{
		return Tuple<T1, T2>((t1.item1+t2.item2) / 2, (t1.item2+t2.item1) / 2);
	}
};

#endif //OTHELLO_H
