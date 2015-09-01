#ifndef RANDOM_H
#define RANDOM_H

#include <cstdlib>
#include "Header.h"

class Rand
{
	unsigned val;
	// src: "http://en.wikipedia.org/wiki/Linear_congruential_generator"
	const static unsigned A = 1103515245;
	const static unsigned C = 12345;
	const static unsigned M = 1u << 31;
public:
	DEF Rand(unsigned seed = 0)
	{
		val = seed % M;
	}

	DEF inline unsigned rand()
	{
		return val = (A * val + C) % M;
	}
	
	DEF inline unsigned getMaxValue() const
	{
		return M;
	}
};

template <typename T, typename FL = float> class Random : public Rand
{
public:
	DEF Random(unsigned seed = 0)
	  : Rand(seed) { }
	
	// Zwraca wartoœæ z przedzia³u <0,1)
	DEF inline T getValue()
	{
		return static_cast<T>(rand()) / getMaxValue();
	}
	
	DEF inline T getValue(T max)
	{
		return static_cast<T>((FL)rand()*max / getMaxValue());
	}
	
	DEF inline T getValue(T min, T max)
	{
		return getValue(max - min) + min;
	}
};

#endif //RANDOM_H

