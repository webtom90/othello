#ifndef VECTOR_H
#define VECTOR_H

#include "Header.h"
#include "Array.h"

// Nieuporz¹dkowana lista liczb.
template <typename T, int SIZE> class Vector : public Array<T, SIZE>
{
public:
	DEF Vector() { }

	DEF Vector(const Vector &other)
	{
		memcpy(this, &other, sizeof(Vector));
	}

	// Dodaje element na koñcu listy.
	DEF void add(T value)
	{
		this->array[this->n++] = value;
	}
};

#endif //VECTOR_H
