#ifndef WATCH_H
#define WATCH_H

//#define ACCURATE_WATCH

#ifdef ACCURATE_WATCH
#include <sys/time.h>
#else
#include <time.h>
#endif

template <typename T> class Watch
{
#ifdef ACCURATE_WATCH
	struct timeval begin;
	struct timeval end;
#else
	clock_t begin;
	clock_t end;
#endif
	T res;
public:
	Watch()
	{
		start();
	}

	void start()
	{
#ifdef ACCURATE_WATCH
		gettimeofday(&begin, NULL);
#else
		begin = clock();
#endif
	}

	T stop()
	{
#ifdef ACCURATE_WATCH
		gettimeofday(&end, NULL);
		int64_t t = end.tv_usec + 1000000LL * end.tv_sec;
		t -= begin.tv_usec + 1000000LL * begin.tv_sec;
		res = static_cast<T>(t) / 1000000LL;
#else
		end = clock();
		res = static_cast<T> (end - begin) / CLOCKS_PER_SEC;
#endif
		return res;
	}

	T operator()(int mul = 1)
	{
		return res*mul;
	}

	T current()
	{
		#ifdef ACCURATE_WATCH
		struct timeval c;
		gettimeofday(&c, NULL);
		int64_t t = c.tv_usec + 1000000LL * c.tv_sec;
		t -= begin.tv_usec + 1000000LL * begin.tv_sec;
		return static_cast<T>(t) / 1000000LL;
		#else
		clock_t c = clock();
		return static_cast<T> (c - begin) / CLOCKS_PER_SEC;
		#endif
	}
};

#endif //WATCH_H
