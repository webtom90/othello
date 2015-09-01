#ifndef TUPLE_H
#define TUPLE_H

#include "Header.h"

// Przechowuje parê wartoœci.
template<typename T1, typename T2> struct Tuple
{
	T1 item1;
	T2 item2;

	DEF Tuple(T1 item1, T2 item2)
	{
		this->item1 = item1;
		this->item2 = item2;
	}
};

#endif //TUPLE_H
