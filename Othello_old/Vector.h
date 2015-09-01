#ifndef VECTOR_H
#define VECTOR_H

#include "Header.h"
#include "Array.h"

// Nieuporz�dkowana lista liczb.
template <typename T, int SIZE> class Vector : public Array<T, SIZE>
{
public:
	DEF Vector() { }

	// Dodaje element na ko�cu listy.
	DEF void add(T value)
	{
		this->array[this->n++] = value;
	}
};

#endif //VECTOR_H
