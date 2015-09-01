#ifndef BOARD_H
#define BOARD_H

#include "Header.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SparseSet.h"
#include "Vector.h"
#include "Tuple.h"
#include <assert.h>

#include <limits.h>
#include <float.h>

// Obs�uguje plansz�, przechowuje jej stan i przeprowadza na niej wszelkie operacje.
class Board
{
public:
	// Typ zwracany przez funkcj� oceny stanu
	typedef float EVALUATION_TYPE;

	const static int WORSE_EVAL = INT_MIN;
	const static int MAX_WEIGHT_VALUE = 1000000;
	// Typ pojedynczego pola planszy.
	typedef char BOARD_ELEMENT_TYPE;
	// Typ indeksu pola.
	typedef char INDEX_TYPE;
	// Maksymalna liczba mo�liwych do wykonania ruch�w.
	const static int MAX_CH_POS = 62;
	// Rozmiar planszy.
	const static int SIZE = 100;
	// Liczba mo�liwych kierunk�w ruchu.
	const static int N_DIR = 8;
	// Szeroko��/wysoko�� planszy.
	const static int WIDTH = 10;

	// Warto�� oznaczj�ca czarny pion.
	const static BOARD_ELEMENT_TYPE BLACK = -1;
	// Warto�� oznaczj�ca bia�y pion.
	const static BOARD_ELEMENT_TYPE WHITE = 1;
	// Warto�� oznaczj�ca puste pole.
	const static BOARD_ELEMENT_TYPE EMPTY = 0;
	// Warto�� oznaczj�ca �cian�.
	const static BOARD_ELEMENT_TYPE WALL = 2;

	// Tworzy now� plansz� z pocz�tkowym ustawieniem pion�w.
	DEF Board()
	{
		empty();
	}

	DEF Board(const Board &board)
	{
		copy(&board);
	}

	DEF Board(BOARD_ELEMENT_TYPE *values)
	{
		setValues(values);
	}

	DEF void empty()
	{
		memset(board + WIDTH + 1, 0, sizeof(board[0]) * (SIZE - 2 * (WIDTH + 1)));

		for(int i = 0; i < 9; i++)
		{
			setValue(0, i, WALL);
			setValue(i, WIDTH - 1, WALL);
			setValue(WIDTH - 1, i + 1, WALL);
			setValue(i + 1, 0, WALL);
		}

		setValue(4, 4, WHITE);
		setValue(5, 5, WHITE);

		setValue(4, 5, BLACK);
		setValue(5, 4, BLACK);

		nPawns = 4;

		getDirections(directions);

		inverted = false;
	}

	DEF void setValues(BOARD_ELEMENT_TYPE *values)
	{
		memset(board + WIDTH + 1, 0, sizeof(board[0]) * (SIZE - 2 * (WIDTH + 1)));

		for (int i = 0; i < 9; i++)
		{
			setValue(0, i, WALL);
			setValue(i, WIDTH - 1, WALL);
			setValue(WIDTH - 1, i + 1, WALL);
			setValue(i + 1, 0, WALL);
		}

		int count = 0;

		for (int i = 0; i < 64; i++)
		{
			int x = i % 8 + 1;
			int y = i / 8 + 1;
			setValue(x, y, values[i]);
			if (values[i] != Board::EMPTY)
                count++;
		}

		this->nPawns = count;

		getDirections(directions);

		inverted = false;
	}

	DEF void copy(const Board *board)
	{
		memcpy(this, board, sizeof(Board));
	}

	// Zwraca element na planszy znajduj�cy si� na wskazanej pozycji.
	DEF BOARD_ELEMENT_TYPE getValue(int x, int y)
	{
		return board[getIndex(x, y)];
	}

	// Zwraca element na planszy znajduj�cy si� pod wskazanym indeksem.
	DEF BOARD_ELEMENT_TYPE getValue(INDEX_TYPE index)
	{
		/*if (board[index] == WALL)
			return WALL;*/
		return board[index];
	}

	// Ustawia warto�� elementu na planszy znajduj�go si� na wskazanej pozycji.
	DEF void setValue(int x, int y, BOARD_ELEMENT_TYPE value)
	{
		board[getIndex(x, y)] = value;
	}

	// Ustawia warto�� elementu na planszy znajduj�go si� pod wskazanym indeksem.
	DEF void setValue(INDEX_TYPE index, BOARD_ELEMENT_TYPE value)
	{
		board[index] = value;
	}

	// Zwraca indeks odpowiadaj�cy wskazanej pozycji.
	DEF static INDEX_TYPE getIndex(INDEX_TYPE x, INDEX_TYPE y)
	{
		return y * WIDTH + x;
	}

	// Zwraca wsp�rz�dn� X pola.
	DEF static INDEX_TYPE getX(INDEX_TYPE move)
	{
		return move % WIDTH;
	}

	// Zwraca wsp�rz�dn� Y pola.
	DEF static INDEX_TYPE getY(INDEX_TYPE move)
	{
		return move / WIDTH;
	}

	DEF static void getMoveX(INDEX_TYPE move, INDEX_TYPE *x)
	{
		*x = move % WIDTH;
		if (*x > 1)
			*x -= WIDTH;
		else if (*x < -1)
			*x += WIDTH;
	}

	DEF static void getMoveXY(INDEX_TYPE move, INDEX_TYPE *x, INDEX_TYPE *y)
	{
		getMoveX(move, x);
		*y = (move - *x) / WIDTH;
	}

	// Umieszcza w zbiorze mo�liwe do wykonania (niekoniecznie poprawne) ruchy.
	DEF void possibleMoves(SparseSet<BOARD_ELEMENT_TYPE, 100, SIZE> &set)
	{
		set.clear();
		for(int y = 1; y < WIDTH - 1; y++)
		{
			int endIndex = getIndex(WIDTH - 1, y);
			for(INDEX_TYPE index = getIndex(1, y); index < endIndex; index++)
			{
				BOARD_ELEMENT_TYPE val = getValue(index);
				if (val != WHITE && val != BLACK)
					continue;

				possibleMoves(set, index);
			}
		}
	}

	// Uaktualnia zbi�r mo�liwych ruch�w po wykonaniu ruchu.
	DEF void updatePossibleMoves(SparseSet<BOARD_ELEMENT_TYPE, 100, SIZE> &set, INDEX_TYPE move)
	{
		set.remove(move);
		possibleMoves(set, move);
	}

	// Okre�la, czy na podanym polu jest mo�liwy ruch.
	DEF static bool isPlayable(int move)
	{
		int x = getX(move);
		int y = getY(move);
		if (x < 1 || x >= (WIDTH - 1))
			return false;

		if (y < 1 || y >= (WIDTH - 1))
			return false;

		return true;
	}

	// Umieszcza w zbiorze poprawne w danej chwili ruchy.
	DEF void validMoves(Vector<BOARD_ELEMENT_TYPE, SIZE> &moves, SparseSet<BOARD_ELEMENT_TYPE, 100, SIZE> &possible, BOARD_ELEMENT_TYPE player)
	{
		moves.clear();

		for(int i = 0; i < possible.size(); i++)
		{
			if (isMoveValid(possible[i], player))
			{
				moves.add(possible[i]);
			}
		}
	}

	// Zwraca kolekcj� z kierunkami.
	DEF const Vector<Board::INDEX_TYPE, N_DIR>& getDirections()
	{
		return directions;
	}

	// Uzupe�nia kolekcj� mo�liwymi kierunkami.
	DEF static void getDirections(Vector<Board::INDEX_TYPE, N_DIR> &dir)
	{
		dir.clear();
		dir.add(-WIDTH - 1);
		dir.add(-WIDTH);
		dir.add(-WIDTH + 1);
		dir.add(-1);
		dir.add(1);
		dir.add(WIDTH - 1);
		dir.add(WIDTH);
		dir.add(WIDTH + 1);
	}

	// Wykonuje ruch umieszczaj�c na planszy pod wskazanym indeksem pion aktualnego gracza.
	DEF void makeMove(INDEX_TYPE index, BOARD_ELEMENT_TYPE player/*, bool invertion*/)
	{
		positions.clear();
		simulateMove(&positions, index, player);
		makeMovePos(&positions, player);
	}

	// Wykonuje ruch umieszczaj�c na planszy pod wskazanym indeksem pion aktualnego gracza.
	// positions - indeksy p�l, na kt�rych pojawiaj� si� piony gracza
	template <typename T>
	DEF void makeMovePos(const T *positions, BOARD_ELEMENT_TYPE player/*, bool inevertion*/)
	{
		performMove(positions, player);

		/*if (inevertion)
			invert();*/

        nPawns++;
	}

	// Pomija kolejk� aktualnego gracza.
	DEF void skipMove(bool invertion)
	{
		if (invertion)
			invert();
	}

	// Wype�nia kolekcj� indeksami p�l, na kt�rych pojawi� si� piony gracza po wykonaniu wskazanego ruchu.
	template <typename T>
	DEF void simulateMove(T *positions, INDEX_TYPE index, BOARD_ELEMENT_TYPE player)
	{
		positions->clear();
		for (int i = 0; i < directions.size(); i++)
		{
			INDEX_TYPE dir = directions[i];
			INDEX_TYPE pos = index + dir;

			while (getValue(pos) == -player)
			{
				pos += dir;
			}

			if (getValue(pos) == player)
			{
				pos -= dir;
				while (getValue(pos) == -player)
				{
					positions->add(pos);
					pos -= dir;
				}
			}
		}

		positions->add(index);
	}

	// Drukuje plansz� w konsoli.
	DEF void print()
	{
		printf("\n  ");
		for(int x = 0; x < 10; x++)
		{
			printf(" %d ", x);
		}
		printf("\n");

		for(int y = 0; y < 10; y++)
		{
			printf("\n%d ", y);
			for(int x = 0; x < 10; x++)
			{
				printf(" %s ", getChar(board[getIndex(x, y)]));
			}
			printf("\n");
		}
		printf("\n");
	}

	DEF Tuple<int, int> counts()
	{
        int count1 = 0;
		int count2 = 0;
		for(int y = 1; y < WIDTH - 1; y++)
		{
			INDEX_TYPE index = getIndex(1, y);
			for(int x = 1; x < WIDTH - 1; x++)
			{
				BOARD_ELEMENT_TYPE value = board[index];

				if (value == BLACK)
					count1++;
				if (value == WHITE)
					count2++;

				index++;
			}
		}
		return Tuple<int, int>(count1, count2);
	}

	// Zwraca rezultat rozgrywki w postaci ilo�ci pion�w obu graczy.
	DEF Tuple<EVALUATION_TYPE, EVALUATION_TYPE> result()
	{
		int count1 = 0;
		int count2 = 0;
		for(int y = 1; y < WIDTH - 1; y++)
		{
			INDEX_TYPE index = getIndex(1, y);
			for(int x = 1; x < WIDTH - 1; x++)
			{
				BOARD_ELEMENT_TYPE value = board[index];

				if (value == BLACK)
					count1++;
				if (value == WHITE)
					count2++;

				index++;
			}
		}

		EVALUATION_TYPE result1 = 0;
		EVALUATION_TYPE result2 = 0;
		if (count1 == count2)
			result1 = result2 = 0.5;
		else if (count1 > count2)
			result1 = 1;
		else
			result2 = 1;
		if (inverted)
			return Tuple<EVALUATION_TYPE, EVALUATION_TYPE>(result2, result1);

		return Tuple<EVALUATION_TYPE, EVALUATION_TYPE>(result1, result2);
	}

	// Odwraca plansz�.
	DEF void invert()
	{
	    inverted = !inverted;

		for(int y = 1; y < WIDTH - 1; y++)
		{
			int endIndex = getIndex(WIDTH - 1, y);
			for(int index = getIndex(1, y); index < endIndex; index++)
			{
				board[index] = -board[index];
			}
		}
	}

	// Wykonuje wskazany ruch poprzez umieszczenie pion�w gracza na planszy.
	template <typename T>
	DEF void performMove(const T *positions, BOARD_ELEMENT_TYPE player)
	{
		for(int i = 0; i < positions->size(); i++)
		{
			setValue(positions[0][i], player);
		}
	}

	DEF int getNPawns()
	{
        return nPawns;
	}
private:
	// Tablica z elementami planszy.
	BOARD_ELEMENT_TYPE board[SIZE];
	// Liczba pion�w na planszy.
	int nPawns;
	// Kolekcja z indeksami kierunk�w.
	Vector<INDEX_TYPE, N_DIR> directions;

	Vector<BOARD_ELEMENT_TYPE, MAX_CH_POS> positions;

	bool inverted;

	DEF const char * getChar(BOARD_ELEMENT_TYPE value)
	{
		switch (value)
		{
		case WHITE:
			return "X";
		case BLACK:
			return "O";
		case WALL:
		case -WALL:
			return "+";
		case EMPTY:
			return " ";
		default:
			return "?";
		}
	}

	// Umieszcza w zbiorze mo�liwe do wykonania ruchy dla wok� danego pola.
	DEF void possibleMoves(SparseSet<BOARD_ELEMENT_TYPE, 100, SIZE> &set, INDEX_TYPE index)
	{
		int beginIndex = index - WIDTH - 1;
		// trzy pola ponad przetwarzanym polem
		if (getValue(beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}
		if (getValue(++beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}
		if (getValue(++beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}

		// dwa pola w aktualnym wierszu
		if (getValue(index - 1) == EMPTY)
		{
			set.add(index - 1);
		}
		if (getValue(index + 1) == EMPTY)
		{
			set.add(index + 1);
		}

		beginIndex = index + WIDTH - 1;
		// trzy pola pod przetwarzanym polem
		if (getValue(beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}
		if (getValue(++beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}
		if (getValue(++beginIndex) == EMPTY)
		{
			set.add(beginIndex);
		}
	}

	// Sprawdza, czy wskazany ruch jest poprawny.
	DEF bool isMoveValid(INDEX_TYPE index, BOARD_ELEMENT_TYPE player)
	{
		for (int i = 0; i < directions.size(); i++)
		{
			int dir = directions[i];
			int pos = index + dir;

			while (getValue(pos) == -player)
			{
				pos += dir;
			}

			if (getValue(pos) == player && getValue(pos - dir) == -player)
			{
				return true;
			}
		}
		return false;
	}
};

#endif //BOARD_H

