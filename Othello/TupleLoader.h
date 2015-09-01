#ifndef TUPLE_LOADER_H
#define TUPLE_LOADER_H

#include <fstream>
#include <math.h>
#include <string>
#include "Board.h"
#include "Random.h"
#include "Vector.h"

#include "BinaryReader.h"

class OthelloPlayer;

class UniversalLoader
{
public:
	friend UniversalLoader *getCudaLoader(UniversalLoader *loader);
	friend void removeCudaLoader(UniversalLoader *loader);

	static const int N_TUPLE_FORMAT = 0;
	static const int WPC_FORMAT = 1;
	static const int BOARD_FORMAT = 2;
	static const int UNKNOWN_FORMAT = -1;

	DEF UniversalLoader()
	{
		type = -1;
		data = nullptr;
		dataSize = 0;
		nFields = 0;
		fields = nullptr;
		nWeights = 0;
		weights = nullptr;
		nTuples = 0;
		tuples = nullptr;
		nValues = 0;
		values = nullptr;
		maxTuplePerPos = 0;
	}

	DEF ~UniversalLoader()
	{
		clear();
	}

	void save(const std::string &filename)
	{
		std::ofstream writer(filename.c_str(), std::ofstream::binary | std::ofstream::trunc | std::ofstream::out);
		writeValue<int>(writer, type);
		if (type == N_TUPLE_FORMAT)
			saveNTuple(writer);
		else
			printf("Unknown type %d\n", type);
		writer.close();
	}

	static UniversalLoader *getEmptyBoards(int size, int seed)
	{
	    if (size <= 0)
            size = 1;

		UniversalLoader *loader = new UniversalLoader();
		int totalSize = size * 64 * sizeof(Board::BOARD_ELEMENT_TYPE);
		loader->setDataSize(totalSize);
		int offset = 0;
		for (int i = 0; i < size; i++)
		{
			memset(loader->data + offset, 0, 64 * sizeof(Board::BOARD_ELEMENT_TYPE));
			loader->data[offset + 27] = loader->data[offset + 36] = Board::WHITE;
			loader->data[offset + 28] = loader->data[offset + 35] = Board::BLACK;
			offset += 64 * sizeof(Board::BOARD_ELEMENT_TYPE);
		}
		loader->nValues = size;
		loader->values = reinterpret_cast<Board::BOARD_ELEMENT_TYPE *>(loader->data);

		loader->type = BOARD_FORMAT;

		return loader;
	}

	UniversalLoader *getSmallerBoard(int size, int seed)
	{
		if (getType() != BOARD_FORMAT)
			return nullptr;

		UniversalLoader *loader = new UniversalLoader();
		int totalSize = size;
		if (totalSize == 0)
            totalSize = getNValues();
        else if (totalSize < 0)
            totalSize = -totalSize;
		totalSize = totalSize * 64 * sizeof(*values);
		loader->setDataSize(totalSize);

		if (size > 0)
        {
            int offset = 0;
            int tmpSize = getNValues() < size ? getNValues() : size;
            for (int i = 0; i < tmpSize; i++)
            {
                int index = (i + seed) % getNValues();
                memcpy(loader->data + offset, &values[index * 64], 64 * sizeof(*values));
                offset += 64 * sizeof(*values);
            }
            if (size > tmpSize)
            {
                for(int i = tmpSize; i < size; i++)
                {
                    memset(loader->data + offset, 0, 64 * sizeof(Board::BOARD_ELEMENT_TYPE));
                    loader->data[offset + 27] = loader->data[offset + 36] = Board::WHITE;
                    loader->data[offset + 28] = loader->data[offset + 35] = Board::BLACK;
                    offset += 64 * sizeof(Board::BOARD_ELEMENT_TYPE);
                }
            }
            loader->nValues = size;
        }
        else if (size == 0)
        {
            memcpy(loader->data, values, getNValues() * 64 * sizeof(*values));
            loader->nValues = getNValues();
        }
        else // size < 0
        {
            Rand r(seed);
            int offset = 0;
            for (int i = 0; i < -size; i++)
            {
                int index = r.rand() % getNValues();
                memcpy(loader->data + offset, &values[index * 64], 64 * sizeof(*values));
                offset += 64 * sizeof(*values);
            }
            loader->nValues = -size;
        }

		loader->values = reinterpret_cast<Board::BOARD_ELEMENT_TYPE *>(loader->data);
		loader->type = type;

		return loader;
	}

	bool load(const std::string &filename)
	{
		clear();

		BinaryReader reader;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!reader.readFromFile(file))
		{
			printf("Nie moge odczytac pliku: %s\n", filename.c_str());
			return false;
		}
		file.close();

		reader.read(type);

		if (type == N_TUPLE_FORMAT)
		{
			loadNTuple(reader);
		}
		else if (type == WPC_FORMAT)
		{
			loadWPC(reader);
		}
		else if (type == BOARD_FORMAT)
		{
			loadBoard(reader);
		}
		else
		{
			printf("Nierozpoznany typ formatu %d\n", type);
			type = UNKNOWN_FORMAT;
		}

		if (!reader.isReaded())
		{
			if (reader.dataLeft() > 0)
				printf("Plik '%s' zawieral niepotrzebne dane\n", filename.c_str());
			else
				printf("Plik '%s' zawieral zbyt malo danych\n", filename.c_str());

			return false;
		}

		return true;
	}

	DEF int getType()
	{
		return type;
	}

	DEF int getNFields()
	{
		return nFields;
	}

	DEF Board::INDEX_TYPE *getFields()
	{
		return fields;
	}

	DEF int getNWeights()
	{
		return nWeights;
	}

	DEF Board::EVALUATION_TYPE *getWeights()
	{
		return weights;
	}

	DEF int getNTuples()
	{
		return nTuples;
	}

	DEF int *getTuples()
	{
		return tuples;
	}

	DEF int getNValues()
	{
		return nValues;
	}

	DEF Board::BOARD_ELEMENT_TYPE *getValues()
	{
		return values;
	}

	DEF char *getData()
	{
		return data;
	}

	DEF int getMaxTuplePerPos()
	{
		return maxTuplePerPos;
	}
private:
	int type;
	char *data;
	int dataSize;
	int nFields;
	Board::INDEX_TYPE *fields;
	int nWeights;
	Board::EVALUATION_TYPE *weights;
	int nTuples;
	int *tuples;
	int nValues;
	Board::BOARD_ELEMENT_TYPE *values;
	int maxTuplePerPos;

	DEF void clear()
	{
		dataSize = 0;
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}

		nFields = 0;
		fields = nullptr;

		nWeights = 0;
		weights = nullptr;

		nTuples = 0;
		tuples = nullptr;

		nValues = 0;
		values = nullptr;

		maxTuplePerPos = 0;
	}

	void loadNTuple(BinaryReader &reader)
	{
		reader.read(nFields);
		reader.read(nWeights);
		reader.read(nTuples);

		int totalSize = nFields * sizeof(*fields);
		totalSize += nWeights * sizeof(*weights);
		totalSize += 3 * nTuples * sizeof(*tuples);
		setDataSize(totalSize);

		int offset(0);
		copyPtr<int>(reader, offset, &fields, nFields);
		copyPtr<double>(reader, offset, &weights, nWeights);
		copyPtr<int>(reader, offset, &tuples, 3 * nTuples);

		int fieldCounts[100] = { 0 };
		for (int i = 0; i < nTuples; i++)
		{
			int index = tuples[i * 3 + 1];
			for (int j = 0; j < tuples[i * 3]; j++)
				fieldCounts[fields[index + j]]++;
		}

		for (int i = 0; i < 100; i++)
		{
			if (maxTuplePerPos < fieldCounts[i])
				maxTuplePerPos = fieldCounts[i];
		}
	}

	void saveNTuple(std::ofstream &writer)
	{
		writeValue<int>(writer, nFields);
		writeValue<int>(writer, nWeights);
		writeValue<int>(writer, nTuples);

		writeArray<int>(writer, fields, nFields);
		writeArray<double>(writer, weights, nWeights);
		writeArray<int>(writer, tuples, 3 * nTuples);
	}

	void loadWPC(BinaryReader &reader)
	{
		int offset(0);

		nWeights = 64;
		int totalSize = nWeights * sizeof(*weights);
		setDataSize(totalSize);
		copyPtr<double>(reader, offset, &weights, nWeights);
	}

	void loadBoard(BinaryReader &reader)
	{
		int offset(0);

		reader.read(nValues);
		int totalSize = nValues * 64 * sizeof(*values);
		setDataSize(totalSize);
		copyPtr<int>(reader, offset, &values, 64 * nValues);
	}

	void setDataSize(int size)
	{
		dataSize = size;
		data = new char[dataSize];
	}

	template<typename SRC_TYPE, typename DST_TYPE>
	void copyPtr(BinaryReader &reader, int &offset, DST_TYPE **pointer, int size)
	{
		*pointer = reinterpret_cast<DST_TYPE *>(data + offset);

		for (int i = 0; i < size; i++)
		{
			SRC_TYPE value = 0;
			reader.read(value);
			(*pointer)[i] = static_cast<DST_TYPE>(value);
		}

		offset += size * sizeof(DST_TYPE);
	}

	template<typename DST_TYPE, typename SRC_TYPE>
	void writeValue(std::ofstream &writer, SRC_TYPE srcValue)
	{
		DST_TYPE value = static_cast<DST_TYPE>(srcValue);
		writer.write(reinterpret_cast<char *>(&value), sizeof(DST_TYPE));
	}

	template<typename DST_TYPE, typename SRC_TYPE>
	void writeArray(std::ofstream &writer, SRC_TYPE *array, int size)
	{
		for (int i = 0; i < size; i++)
		{
			DST_TYPE value = static_cast<DST_TYPE>(array[i]);
			writer.write(reinterpret_cast<char *>(&value), sizeof(DST_TYPE));
		}
	}

	template <typename T>
	void setPtr(int &offset, T **pointer, int size)
	{
		size_t elementSize = sizeof(T);
		if (dataSize < size * elementSize + offset)
		{
			printf("Koniec pliku!");
			exit(-1);
		}
		*pointer = reinterpret_cast<T *>(data + offset);
		offset += size * elementSize;
	}
};

#endif // TUPLE_LOADER_H
