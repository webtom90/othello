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

template <typename T> class Random : public Rand
{
public:
	DEF Random(unsigned seed = 0)
	  : Rand(seed) { }
	 
	DEF inline T getValue()
	{
		return static_cast<T>(rand()) / getMaxValue();
	}
	 
	DEF inline T getValue(T max)
	{
		return getValue() * max;
	}
	 
	DEF inline T getValue(T min, T max)
	{
		return getValue(max - min) + min;
	}
};

/*
Randoms compare
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define DEF
#include "Random.h"
#include "Watch.h"

int main()
{
  Watch<float> w;
  const int IT = 100;
  float results[IT] = { 0 };
  for(int it = 0; it < IT; it++)
  {
    if ((it % (IT/10)) == 0)
      printf("%d/%d\n", it, IT);
  Random<float> random(it);
  srand(it);
  const int N = 100;
  int tab[N] = { 0 };
  for(int i = 0; i < 100000 * N; i++)
  {
    tab[random.rand()%N]++;
    //tab[rand()%N]++;
  }
  double avg(0), dev(0);
  for(int i = 0; i < N; i++)
  {
    avg += tab[i] / (double)N;
  }
  for(int i = 0; i < N; i++)
  {
    dev += (tab[i]-avg)*(tab[i]-avg);
  }
  dev /= N;
  dev = sqrt(dev);
  results[it] = dev;
  }
  
  double favg(0), fdev(0);
  for(int i = 0; i < IT; i++)
  {
    favg += results[i] / (double)IT;
  }
  for(int i = 0; i < IT; i++)
  {
    fdev += (results[i]-favg)*(results[i]-favg);
  }
  fdev /= IT;
  fdev = sqrt(fdev);
  
  w.stop();
  printf("%.1f\t%.3f\n", favg, fdev);
  printf("%.3f\n", w());
  return 0;
}
*/

#endif //RANDOM_H

