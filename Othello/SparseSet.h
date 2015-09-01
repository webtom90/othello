#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include "Header.h"

template <typename ELEMENT_TYPE, int MAX_VALUE, int MAX_SIZE, typename INDEX_TYPE = int> class SparseSet
{
protected:
	ELEMENT_TYPE dense[MAX_SIZE];
	INDEX_TYPE sparse[MAX_VALUE];
	// Aktualna liczba przechowywanych elementów.
	INDEX_TYPE n;
public:
	DEF SparseSet()
	{
		n = 0;
	}

	DEF void clear()
	{
		n = 0;
	}

	DEF INDEX_TYPE size() const
	{
		return n;
	}

	DEF INDEX_TYPE maxSize() const
	{
		return MAX_SIZE;
	}

	DEF bool add(ELEMENT_TYPE value)
	{
		if (contains(value))
			return false;

		sparse[value] = n;
		dense[n] = value;
		n++;

		return true;
	}

	DEF bool remove(ELEMENT_TYPE value)
	{
		if (!contains(value))
			return false;

		INDEX_TYPE slot = sparse[value];
		INDEX_TYPE last = n - 1;

		if (slot < last)
		{
			ELEMENT_TYPE lastVal = dense[last];
			dense[slot] = lastVal;
			sparse[lastVal] = slot;
		}

		n--;
		return true;
	}

	DEF bool contains(ELEMENT_TYPE value) const
	{
		return ((unsigned)sparse[value] < n && dense[sparse[value]] == value);
	}

	DEF ELEMENT_TYPE operator[](INDEX_TYPE index) const
	{
		return dense[index];
	}
};

#endif //SPARSE_SET_H

