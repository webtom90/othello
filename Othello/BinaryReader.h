#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <fstream>
#include <string>

struct Data
{
	Data(char *data, unsigned size)
	{
		this->data = data;
		this->size = size;
	}

	char *getData()
	{
		return data;
	}

	unsigned getSize()
	{
		return size;
	}
private:
	char *data;
	unsigned size;
};

class BinaryReader
{
public:
	BinaryReader()
	{
		data = nullptr;
		dataSize = 0;
		offset = -1;
	}

	~BinaryReader()
	{
		clear();
	}

	bool readFromFile(std::ifstream &file)
	{
		clear();

		if (file.is_open())
		{
			file.seekg(0, std::ios::end);
			dataSize = file.tellg();
			data = new char[dataSize];
			file.seekg(0, std::ios::beg);
			file.read(data, dataSize);

			offset = 0;

			return true;
		}
		else
			return false;
	}

	template <typename T>
	bool read(T &value)
	{
		if (!good())
			return false;

		size_t size = sizeof(T);
		if (dataSize < size + offset)
		{
			printf("Koniec readera!\n");
			return false;
		}
		memcpy(&value, data + offset, size);
		offset += size;
		return true;
	}

	int read(char *buffer, int length)
	{
	    int readed = length;
	    if (dataSize < offset + length)
            readed = dataSize - offset;

        memcpy(buffer, data + offset, readed);
        offset += readed;

	    return readed;
	}

	bool isReaded()
	{
		return dataSize == offset;
	}

	int dataLeft()
	{
		if (!good())
			return -1;

		return dataSize - offset;
	}

	static Data *getData(std::string filename)
	{
		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		BinaryReader reader;
		if (!reader.readFromFile(file))
			return nullptr;

		char *data = reader.data;
		unsigned size = reader.dataSize;
		reader.data = nullptr;

		return new Data(data, size);
	}

	Data copyData()
	{
		char *data = new char[dataSize];
		memcpy(data, this->data, dataSize);
		return Data(data, dataSize);
	}
private:
	DEF void clear()
	{
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}
		dataSize = 0;
		offset = -1;
	}

	bool good()
	{
		return offset >= 0;
	}

	char *data;
	int dataSize;
	int offset;
};

class BinWriter
{
public:
    static bool writeToFile(std::string filename, char *data, int dataSize)
    {
        std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
        bool result = writeToFile(file, data, dataSize);
        file.close();
        return result;
    }

    static bool writeToFile(std::ofstream &file, char *data, int dataSize)
	{
	    file.write(data, dataSize);
	    return true;
	}
};

#endif // BINARY_READER_H
