#ifndef VECTOR_H
#define VECTOR_H

#include "Header.h"
#include "Array.h"

// Nieuporządkowana lista liczb.
template <typename T, int SIZE> class Vector : public Array<T, SIZE>
{
public:
	DEF Vector() { }

	DEF Vector(const Vector &other)
	{
		memcpy(this, &other, sizeof(Vector));
	}

	// Dodaje element na końcu listy.
	DEF void add(T value)
	{
		this->array[this->n++] = value;
	}
};

#endif //VECTOR_H
