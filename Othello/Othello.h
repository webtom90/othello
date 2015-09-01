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
	DEF Othello(OthelloPlayer *player, OthelloPlayer **experts, int nExperts, int seed) :
		random(seed)
	{
		OthelloPlayer **newPlayers = new OthelloPlayer*[nExperts + 1];
		newPlayers[0] = player;
		for(int i = 1; i <= nExperts; i++)
			newPlayers[i] = experts[i-1];
		init(newPlayers, nExperts + 1);
	}

	DEF Othello(OthelloPlayer **players, int nPlayers, int seed) :
		random(seed)
	{
		OthelloPlayer **newPlayers = new OthelloPlayer*[nPlayers];
		for(int i = 0; i < nPlayers; i++)
			newPlayers[i] = players[i];
		init(newPlayers, nPlayers);
	}

	DEF ~Othello()
	{
		delete data;
		for(int i = 0; i < nPlayers; i++)
			delete params[i];
		delete[] params;
		delete[] players;
	}

	// Rozgrywa parê gier.
	DEF Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> playDouble(int p1, int p2)
	{
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result1 = play(p1, p2);
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result2 = play(p2, p1);
		return aggregate(result1, result2);
	}

	DEF Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> playDouble(Board *baseBoard, int p1, int p2)
	{
		board.copy(baseBoard);
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result1 = play(&board, p1, p2);
		board.copy(baseBoard);
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result2 = play(&board, p2, p1);
		return aggregate(result1, result2);
	}

	DEF Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> play(int p1, int p2)
	{
		board.empty();
		Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result = play(&board, p1, p2);
		return result;
	}

	// Rozgrywa pojedyncz¹ grê.
	DEF Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> play(Board *board, int p1, int p2)
	{
		OthelloPlayer *player1 = players[p1];
		OthelloPlayer *player2 = players[p2];
		PlayerParams* playersParams[] = { params[p1], params[p2] };
		return play(board, player1, player2, playersParams, &validMoves, &possibleMoves, data, &random);
	}

	DEF static Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> playDouble(Board &board, OthelloPlayer *player1, OthelloPlayer *player2, int seed)
    {
        Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> result1(0, 0), result2(0, 0);
        {
            Board b1(board);
            result1 = play(b1, player1, player2, seed);
        }
        {
            Board b1(board);
            result2 = play(b1, player2, player1, seed + 1);
        }
        return aggregate(result1, result2);
    }

	DEF static Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> play(Board &board, OthelloPlayer *player1, OthelloPlayer *player2, int seed)
	{
	    Rand random(seed);
		Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> validMoves;
        SparseSet<Board::BOARD_ELEMENT_TYPE, 100, Board::SIZE> possibleMoves;
        GameData data;
		PlayerParams* playersParams[] = { player1->getPlayerParams(random.rand()), player2->getPlayerParams(random.rand()) };

		auto result = play(&board, player1, player2, playersParams, &validMoves, &possibleMoves, &data, &random);
		//play(Board *board, OthelloPlayer *player1, OthelloPlayer *player2, PlayerParams* playersParams[], Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, SparseSet<Board::BOARD_ELEMENT_TYPE, 100, Board::SIZE> *possibleMoves, GameData *data, Rand *random)

        for(int i = 0; i < 2; i++)
            delete playersParams[i];

		return result;
	}

	template <typename T1, typename T2>
	DEF static Tuple<T1, T2> aggregate(const Tuple<T1, T2> &t1, Tuple<T1, T2> &t2)
	{
		return Tuple<T1, T2>((t1.item1+t2.item2) / 2, (t1.item2+t2.item1) / 2);
	}

	DEF Board &getBoard()
	{
		return board;
	}
private:
	DEF void init(OthelloPlayer **players, int nPlayers)
	{
		this->players = players;
		this->nPlayers = nPlayers;
		params = new PlayerParams*[nPlayers];
		for(int i = 0; i < nPlayers; i++)
			params[i] = players[i]->getPlayerParams(random.rand());
        data = new GameData();
	}

	OthelloPlayer **players;
	PlayerParams **params;
	GameData *data;
	int nPlayers;

	Rand random;

	Board board;
	Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> validMoves;
	SparseSet<Board::BOARD_ELEMENT_TYPE, 100, Board::SIZE> possibleMoves;

	DEF static Tuple<Board::EVALUATION_TYPE, Board::EVALUATION_TYPE> play(Board *board, OthelloPlayer *player1, OthelloPlayer *player2, PlayerParams* playersParams[], Vector<Board::BOARD_ELEMENT_TYPE, Board::SIZE> *validMoves, SparseSet<Board::BOARD_ELEMENT_TYPE, 100, Board::SIZE> *possibleMoves, GameData *data, Rand *random)
	{
		Random<float> r(random->rand());
		Random<int> r2(random->rand());
	    validMoves->clear();
		possibleMoves->clear();
		board->possibleMoves(*possibleMoves);
		bool aMoveWasPossible;
		OthelloPlayer* players[] = { player1, player2 };

		do
		{
			aMoveWasPossible = false;
			for (int p = 0; p < 2; p++)
			{
                bool negated = false;
                if (p == 1)
                {
                    negated = player2->isNegated(board);
                }
                bool boardInversion = !negated && p != 0;

				if (boardInversion)
                    board->invert();

				OthelloPlayer *player = players[p];
				PlayerParams *par = playersParams[p];
				Board::BOARD_ELEMENT_TYPE playerColor = (p == 0 || boardInversion) ? Board::BLACK : Board::WHITE;
				board->validMoves(*validMoves, *possibleMoves, playerColor);
				if (validMoves->size() == 0)
				{
					if (boardInversion)
                        board->invert();
					continue;
				}

				int move = (*validMoves)[0];
				if (player->getRandomMoveFreq() > r.getValue())
				{
					move = (*validMoves)[r2.getValue(validMoves->size())];
				}
				else
				{
					move = player->getMove(board, validMoves, playerColor, negated, par, data);
				}

				board->makeMove(move, playerColor);
				if (boardInversion)
                    board->invert();
				board->updatePossibleMoves(*possibleMoves, move);
				aMoveWasPossible = true;
			}
		}
		while (aMoveWasPossible);

		return board->result();
	}
};

#endif //OTHELLO_H
