#ifndef N_TUPLES1_H
#define N_TUPLES1_H

#include "Board.h"
#include "NTuple.h"
#include "Vector.h"
#include "Random.h"
#include "SortedSet.h"
#include "SparseSet.h"

#define N_TUPLES(SIZE) (SIZE == 1 ? 64 : (SIZE == 2 ? 32 : (SIZE == 3 ? 24 : SIZE == 4 ? 21 : -1)))

#define N_WEIGHTS(SIZE) WEIGHTS_PER_TUPLE(SIZE) * N_TUPLES(SIZE)

#define N_MAX_TUPLES_PER_FIELD(SIZE) (SIZE == 1 ? 1 : (SIZE == 2 ? 14 : (SIZE == 3 ? 16 : SIZE == 4 ? 24 : -1)))

template <int N> class NTuples
{
public:
	// Maksymalna liczba symetrycznych odbić jednego punktu.
	const static int N_SIM = 8;

	DEF NTuples()
	{
		generateValues();
		init();
	}

	DEF Board::EVALUATION_TYPE getValue(Board &board, Board::INDEX_TYPE move)
	{
		Board tmpBoard(board);
		SparseSet<Board::BOARD_ELEMENT_TYPE, Board::SIZE, Board::MAX_CH_POS> p;
		tmpBoard.simulateMove(p, move);
		SparseSet<unsigned short, N_TUPLES(N)*N_SIM, N_TUPLES(N)*N_SIM> indexes;
		for(int i = 0; i < p.size(); i++)
		{
			for(int j = 0; j < tuplesInPos[p[i]-(Board::WIDTH + 1)].size(); j++)
				indexes.add(tuplesInPos[p[i]-(Board::WIDTH + 1)][j]);
		}

		Board::EVALUATION_TYPE result = 0;
		for(int i = 0; i < indexes.size(); i++)
		{
			result -= tuples[indexes[i]].getValue(tmpBoard);
		}
		tmpBoard.makeMove(move, p);
		for(int i = 0; i < indexes.size(); i++)
		{
			result += tuples[indexes[i]].getValue(tmpBoard);
		}

		return result;
	}
private:
	Board::EVALUATION_TYPE weights[N_WEIGHTS(N)];
	Vector<NTuple<N>, N_TUPLES(N)*N_SIM> tuples;
	Vector<Vector<int, N_MAX_TUPLES_PER_FIELD(N)>, Board::SIZE - 2 * (Board::WIDTH + 1)> tuplesInPos;

	DEF void init()
	{
		SortedSet<SortedSet<Board::INDEX_TYPE, N>, N_TUPLES(N)> sets;
		getTuples(sets);
		for (int i = 0; i < sets.size(); i++)
		{
			Vector<Vector<Board::INDEX_TYPE, N>, N_SIM> locations;
			getSim(locations, sets[i]);

			for(int j = 0; j < locations.size(); j++)
				tuples.add(NTuple<N>(locations[j], &weights[i * WEIGHTS_PER_TUPLE(N)]));
			/*for(int a = 0; a < locations.size(); a++)
			{
				if (locations[a][0]==11 && locations[a][locations[i].size()-1]==44)
				{
					printf("Found %d\n", i);
					for(int j = 0; j < sets[i].size(); j++)
						printf("%d ", sets[i][j]);
					printf("\n\n");
					for(int j = 0; j < locations.size(); j++)
					{
						for(int k = 0; k < locations[j].size(); k++)
							printf("%d ", locations[j][k]);
						printf("\n");
					}

					printf("\n");
				}
			}*/
		}

		tuplesInPos.clear();
		for(int i = 0; i < tuplesInPos.maxSize(); i++)
		{
			tuplesInPos.add(Vector<int, N_MAX_TUPLES_PER_FIELD(N)>());
			for(int j = 0; j < tuples.size(); j++)
			{
				if (tuples[j].contains(i + Board::WIDTH + 1))
				{
					tuplesInPos.getReference(i).add(j);
				}
			}
		}
		/*printf("Size: %d\n", tuples.size());
		for(int i = 0; i < tuples.size(); i++)
		{
			for(int j = 0; j < N; j++)
				printf("%d ", tuples[i].locations[j]);
			if (((i+1) % 4) == 0)
				printf("\n");
			else
				printf("\t");
		}*/
	}

	DEF void generateValues()
	{
		Random<Board::EVALUATION_TYPE> random;
		for(int i = 0; i < N_WEIGHTS(N); i++)
		{
			weights[i] = random.getValue(-0.2f, 0.2f);
		}
	}

	DEF void getTuples(SortedSet<SortedSet<Board::INDEX_TYPE, N>, N_TUPLES(N)> &sets)
	{
		Vector<Board::INDEX_TYPE, Board::N_DIR> dir;
		Board::getDirections(dir);

		sets.clear();

		for (Board::INDEX_TYPE x = 1; x < Board::WIDTH / 2; x++)
		{
			for (Board::INDEX_TYPE y = 1; y <= x; y++)
			{
				Board::INDEX_TYPE move = Board::getIndex(x, y);
				for (Board::INDEX_TYPE d = 0; d < dir.size(); d++)
				{
					if (dir[d] < 0)
						continue;

					if (!Board::isPlayable(move + (N - 1) * dir[d]))
						continue;

					SortedSet<Board::INDEX_TYPE, N> currentSet;
					Board::INDEX_TYPE currentMove = move;

					for (int i = 0; i < N; i++)
					{
						currentSet.add(currentMove);
						currentMove += dir[d];
					}

					currentMove = currentSet[currentSet.size() - 1];
					if (!isBest(currentSet))
						continue;
					#if OUTPUT_MODE
					if (sets.add(currentSet))
					{
						for (int i = 0; i < currentSet.size(); i++)
							printf("%d ", currentSet[i]);

						Board::INDEX_TYPE dx, dy;
						Board::getMoveXY(dir[d], &dx, &dy);

						printf("(%2d,%2d)\n", dx, dy);
					}
					else
					{
						printf("NTuple rejected: ");
						for (int i = 0; i < currentSet.size(); i++)
							printf("%d ", currentSet[i]);
						printf("\n");
					}
					#else
					/*if (sets.size() == 21)
					{
						printf("Content:\n");
						for(int it = 0; it < sets.size(); it++)
						{
							for(int i2 = 0; i2 < sets[it].size(); i2++)
								printf("%d ", sets[it][i2]);
							printf("\n");
						}
						printf("Adding:\n");
						for(int i2 = 0; i2 < currentSet.size(); i2++)
							printf("%d ", currentSet[i2]);
						printf("\n");
					}*/
					sets.add(currentSet);
					#endif
				}
			}
		}

		#if OUTPUT_MODE
		if (sets.size() != N_TUPLES(N))
		{
			printf("Niepoprawna liczba ntupli - %d zamiast %d\n", sets.size(), N_TUPLES(N));
		}
		#endif
		/*for(int i = 0; i < sets.size(); i++)
		{
			for(int j = 0; j < sets[i].size(); j++)
				printf("%d ", sets[i][j]);
			printf("\n");
		}*/
	}

	DEF static void getSim(Vector<Vector<Board::INDEX_TYPE, N>, N_SIM> &sim, const Array<Board::INDEX_TYPE, N> &moves)
	{
		Vector<Vector<Board::INDEX_TYPE, N>, N_SIM> vec;
		SortedSet<SortedSet<Board::INDEX_TYPE, N>, N_SIM> tmpSet;

		getSimmetrics(vec, moves);

		sim.clear();
		for (int i = 0; i < vec.size(); i++)
		{
			SortedSet<Board::INDEX_TYPE, N> set;
			for (int j = 0; j < vec[i].size(); j++)
			{
				set.add(vec[i][j]);
			}

			if (tmpSet.add(set))
				sim.add(vec[i]);
		}
	}

	DEF static void getSimmetrics(Vector<Vector<Board::INDEX_TYPE, N>, N_SIM> &sim, const Array<Board::INDEX_TYPE, N> &moves)
	{
		sim.clear();
		for (int i = 0; i < N_SIM; i++)
		{
			Vector<Board::INDEX_TYPE, N> set;
			sim.add(set);
		}

		for (int i = 0; i < moves.size(); i++)
		{
			Board::INDEX_TYPE move = moves[i];
			Board::INDEX_TYPE x1 = Board::getX(move);
			Board::INDEX_TYPE y1 = Board::getY(move);
			Board::INDEX_TYPE x2 = Board::WIDTH - 1 - x1;
			Board::INDEX_TYPE y2 = Board::WIDTH - 1 - y1;
			SortedSet<Board::INDEX_TYPE, 8> set;

			sim.getReference(0).add(Board::getIndex(x1, y1));
			sim.getReference(1).add(Board::getIndex(x1, y2));
			sim.getReference(2).add(Board::getIndex(x2, y1));
			sim.getReference(3).add(Board::getIndex(x2, y2));
			sim.getReference(4).add(Board::getIndex(y1, x1));
			sim.getReference(5).add(Board::getIndex(y1, x2));
			sim.getReference(6).add(Board::getIndex(y2, x1));
			sim.getReference(7).add(Board::getIndex(y2, x2));
		}
	}

	DEF static void getSimmetrics(SortedSet<SortedSet<Board::INDEX_TYPE, N>, N_SIM> &sim, const SortedSet<Board::INDEX_TYPE, N> &moves)
	{
		Vector<Vector<Board::INDEX_TYPE, N>, N_SIM> vec;

		getSimmetrics(vec, moves);

		sim.clear();
		for (int i = 0; i < vec.size(); i++)
		{
			SortedSet<Board::INDEX_TYPE, N> set;
			for (int j = 0; j < vec[i].size(); j++)
			{
				set.add(vec[i][j]);
			}

			sim.add(set);
		}
	}

	DEF static bool isBest(SortedSet<Board::INDEX_TYPE, N> &set)
	{
		Board::INDEX_TYPE begin = set[0];
		Board::INDEX_TYPE end = set[set.size() - 1];
		if (end < begin)
			return false;
		SortedSet<SortedSet<Board::INDEX_TYPE, N>, N_SIM> tmpSet;
		getSimmetrics(tmpSet, set);
		if (tmpSet[0] < set)
			return false;

		return true;
	}
};

#endif //N_TUPLES1_H

