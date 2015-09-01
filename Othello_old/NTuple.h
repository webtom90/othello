#ifndef N_TUPLE1_H
#define N_TUPLE1_H

#include "Board.h"
#include "Vector.h"
#include "Collections.h"

#define WEIGHTS_PER_TUPLE(SIZE) (SIZE == 1 ? 3 : (SIZE == 2 ? 9 : (SIZE == 3 ? 27 : SIZE == 4 ? 81 : -1)))

template <int N> class NTuple
{
public:
	// Typ indeksu odnosz¹cego siê do tablicy wag.
	typedef short W_INDEX_TYPE;
	// Typ przechowuj¹cy indeksy.
	typedef Vector<Board::INDEX_TYPE, N> LOCATIONS_TYPE;

	DEF NTuple() { }

	DEF NTuple(LOCATIONS_TYPE locations, Board::EVALUATION_TYPE *weights)
	{
		this->locations = locations;
		this->weights = weights;
	}

	DEF Board::EVALUATION_TYPE getValue(Board &board)
	{
		int index = 0;
		int indexBase = 1;
		for (int i = 0; i < locations.size(); i++)
		{
			index += getWeightIndex(board, locations[i]) * indexBase;
			indexBase *= 3;
		}

		return weights[index];
	}

	DEF bool isIn(const SparseSet<Board::BOARD_ELEMENT_TYPE, Board::SIZE, Board::MAX_CH_POS> &set)
	{
		return Collections::containsAny(locations, set);
	}

	DEF bool contains(const Board::INDEX_TYPE field)
	{
		return locations.containsBegin(field);
	}
//private:
	DEF W_INDEX_TYPE getWeightIndex(Board &board, Board::INDEX_TYPE field)
	{
		Board::BOARD_ELEMENT_TYPE value = board.getValue(field);
		#if OUTPUT_MODE
		if (value < -1 || value > 1)
		{
			printf("Niepoprawny indeks pola %d.\n", value);
			exit(-1);
			return -1;
		}
		#endif
		return value + 1;
	}

	LOCATIONS_TYPE locations;
	Board::EVALUATION_TYPE *weights;
};

#endif //N_TUPLE1_H

