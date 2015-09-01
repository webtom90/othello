#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "Vector.h"
#include "SparseSet.h"
#include "SortedSet.h"

#include "Header.h"

namespace Collections
{

// Sprawdza, którykolwiek z elementów set1 znajduje się w set2.
// Iteruje po set1, wywołuje metodę contains w set2.
template<typename T1, typename T2>
DEF bool containsAny(const T1 &set1, const T2 &set2)
{
	if (set1.size() == 0 || set2.size() == 0)
		return false;
	for(unsigned i = 0; i < set1.size(); i++)
	{
		if (set2.contains(set1[i]))
			return true;
	}
	return false;
}

}

#endif //COLLECTIONS_H

