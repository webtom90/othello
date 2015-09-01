#ifndef N_TUPLE_H
#define N_TUPLE_H

#include "Board.h"

class NTuple
{
public:
	DEF NTuple()
	{
		n = 0;
		fields = nullptr;
		weights = nullptr;
	}

	DEF NTuple(unsigned char n, Board::INDEX_TYPE *fields, Board::EVALUATION_TYPE *weights)
	{
		this->n = n;
		this->fields = fields;
		this->weights = weights;
	}

	DEF Board::EVALUATION_TYPE getValue(Board *board)
	{
		int index = 0;
		for (unsigned char i = 0; i < n; i++)
		{
			index *= 3;
			index += getWeightIndex(board, fields[i]);
		}
		return weights[index];
	}

	DEF bool isIn(const SparseSet<Board::BOARD_ELEMENT_TYPE, Board::SIZE, Board::MAX_CH_POS> &set)
	{
		for (int i = 0; i < n; i++)
			if (set.contains(fields[i]))
				return true;

		return false;
	}

	DEF bool contains(const Board::INDEX_TYPE field)
	{
		for (int i = 0; i < n; i++)
			if (fields[i] == field)
				return true;

		return false;
	}
private:
	unsigned char n;
	Board::INDEX_TYPE *fields;
	Board::EVALUATION_TYPE *weights;

	DEF unsigned char getWeightIndex(Board *board, Board::INDEX_TYPE field)
	{
		Board::BOARD_ELEMENT_TYPE value = board->getValue(field);
		return 2 - (value + 1);
	}
};

#endif //N_TUPLE_H
