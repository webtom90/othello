#ifndef N_TUPLES_H
#define N_TUPLES_H

#include "NTuple.h"
#include "Board.h"
#include "Vector.h"
#include "SparseSet.h"
#include "Random.h"
#include "PlayerParams.h"

template <int N_FIELDS, int N_WEIGHTS, int N_TUPLES, int TUPLES_PER_FIELD>
class NTuples
{
public:
	DEF NTuples()
	{
		printf("NTuples()\n");
	}

	/// Tworzy now¹ kolekcjê NTupli,
	/// generuj¹c wszystkie mo¿liwe kombinacje dla zadanej d³ugoœci.
	DEF NTuples(Board::INDEX_TYPE *fields, Board::EVALUATION_TYPE *weights, int *allTuples)
	{
		memcpy(this->weights, weights, sizeof(Board::EVALUATION_TYPE)*N_WEIGHTS);
		memcpy(this->fields, fields, sizeof(Board::INDEX_TYPE)*N_FIELDS);
		for (int i = 0; i < N_TUPLES; i++)
		{
			int *tuples = allTuples + (3 * i);
			int n = tuples[0];
			Board::INDEX_TYPE *f = this->fields + tuples[1];
			Board::EVALUATION_TYPE *w = this->weights + tuples[2];
			this->tuples.add(NTuple(n, f, w));
		}
		
		calculatePos();
	}

	DEF NTuples(const NTuples &nTuples)
	{
		printf("NTuples(const NTuples &nTuples)\n");
	}

	DEF Board::EVALUATION_TYPE getValue(Board *board, Board::INDEX_TYPE move, Board::BOARD_ELEMENT_TYPE player, NTuplePlayerParams<N_TUPLES> *p, GameData *data)
	{
	    p->indexes.clear();
		data->tmpBoard.copy(board);
		data->tmpBoard.simulateMove(&(data->positions), move, player);
		for (int i = 0; i < data->positions.size(); i++)
		{
			for (int j = 0; j < tuplesInPos[data->positions[i] - (Board::WIDTH + 1)].size(); j++)
				p->indexes.add(tuplesInPos[data->positions[i] - (Board::WIDTH + 1)][j]);
		}

		Board::EVALUATION_TYPE result = 0;

		for (int i = 0; i < p->indexes.size(); i++)
			result -= tuples[p->indexes[i]].getValue(&(data->tmpBoard));

		data->tmpBoard.performMove(&(data->positions), player);

		for (int i = 0; i < p->indexes.size(); i++)
			result += tuples[p->indexes[i]].getValue(&(data->tmpBoard));

		return result;
	}

	DEF Board::EVALUATION_TYPE *getWeights()
	{
		return weights;
	}
private:
	Board::INDEX_TYPE fields[N_FIELDS];
	Board::EVALUATION_TYPE weights[N_WEIGHTS];
	Vector<NTuple, N_TUPLES> tuples;
	Vector<Vector<short, TUPLES_PER_FIELD>, Board::SIZE - 2 * (Board::WIDTH + 1)> tuplesInPos;

	DEF void calculatePos()
	{
		tuplesInPos.clear();
		for (int i = 0; i < tuplesInPos.maxSize(); i++)
		{
			tuplesInPos.add(Vector<short, TUPLES_PER_FIELD>());
			for (int j = 0; j < tuples.size(); j++)
			{
				if (tuples[j].contains(getGlobalIndex(i)))
				{
					tuplesInPos.ref(i).add(j);
				}
			}
		}
	}

	DEF Board::INDEX_TYPE getLocalIndex(Board::INDEX_TYPE globalIndex)
	{
		return globalIndex - (Board::WIDTH + 1);
	}

	DEF Board::INDEX_TYPE getGlobalIndex(Board::INDEX_TYPE localIndex)
	{
		return localIndex + (Board::WIDTH + 1);
	}
};

#endif //N_TUPLES_H
