#ifndef ARRAY_H
#define ARRAY_H

#include "Header.h"

// Kolekcja element�w o zadanym z g�ry maksymalnym rozmiarze.
template <typename T, int SIZE> class Array
{
protected:
	// Tablica element�w.
	T array[SIZE];
	// Aktualna liczba przechowywanych element�w.
	int n;
public:
	DEF Array()
	{
		n = 0;
	}

	// Ilo�� element�w w kolekcji.
	DEF int size() const
	{
		return n;
	}
	
	DEF static int maxSize()
	{
		return SIZE;
	}

	// Zwraca warto�� elementu pod wskazanym indeksem.
	DEF T operator[](int index) const
	{
		return array[index];
	}

	DEF T& getReference(int index)
	{
		return array[index];
	}

	// Ustawia warto�� elementu pod wskazanym indeksem.
	DEF void set(int index, int value)
	{
		array[index] = value;
	}

	// Usuwa wszystkie elementy z kolekcji.
	DEF void clear()
	{
		n = 0;
	}

	// Sprawdza istnienie elementu, przeszukuj�c kolekcj� od pocz�tku.
	DEF bool containsBegin(T value) const
	{
		for(int i = 0; i < n; i++)
		{
			if (array[i] == value)
				return true;
		}

		return false;
	}

	// Sprawdza istnienie elementu, przeszukuj�c kolekcj� od ko�ca.
	DEF bool containsEnd(T value) const
	{
		for(int i = n - 1; i >= 0; i--)
		{
			if (array[i] == value)
				return true;
		}

		return false;
	}

	DEF bool operator==(const Array<T, SIZE> &other) const
	{
		if (n != other.n)
		{
			return false;
		}

		for(int i = 0; i < n; i++)
		{
			if (array[i] != other.array[i])
				return false;
		}

		return true;
	}

	DEF bool operator<(const Array<T, SIZE> &other) const
	{
		if (n != other.n)
		{
			return false;
		}

		for (int i = 0; i < n; i++)
		{
			if (array[i] == other.array[i])
				continue;

			if (array[i] < other.array[i])
				return true;
			
			return false;

		}

		return false;
	}
};

#endif //ARRAY_H
