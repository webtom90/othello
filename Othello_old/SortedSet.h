#ifndef SORTED_SET_H
#define SORTED_SET_H

#include "Header.h"
#include "Array.h"

// Posortowany rosn¹co zbiór.
template <typename T, int SIZE> class SortedSet : public Array<T, SIZE>
{
public:
	DEF SortedSet() { }

	// Kopiuje elementy z innej tablicy.
	DEF static void copy(SortedSet<T, SIZE> &set, const Array<T, SIZE> &source)
	{
		set.clear();
		for(int i = 0; i < source.size(); i++)
		{
			set.add(source[i]);
		}
	}

	// Kopiuje elementy z innej tablicy, zoptymalizowane pod k¹tem kolekcji posortowanej malej¹co.
	DEF static void copyBegin(SortedSet<T, SIZE> &set, const Array<T, SIZE> &source)
	{
		set.clear();
		for(int i = source.size() - 1; i > 0; i--)
		{
			set.addEnd(source[i]);
		}
	}

	// Kopiuje elementy z innej tablicy, zoptymalizowane pod k¹tem kolekcji posortowanej rosn¹co.
	DEF static void copyEnd(SortedSet<T, SIZE> &set, const Array<T, SIZE> &source)
	{
		set.clear();
		for(int i = 0; i < source.size(); i++)
		{
			set.addEnd(source[i]);
		}
	}

	// Szuka w kolekcji elementu, korzystaj¹c z wyszukiwania binarnego.
	// Zwraca indeks elementu, je¿eli zwrócona wartoœæ jest >= 0.
	// W przeciwnym wypadku zwraca: -(docelowy_indeks + 1).
	DEF int binarySearch(T value) const
	{
		int imin = 0;
		int imax = this->n - 1;

		int imid = 0;
		while (imax >= imin)
		{
			imid = (imax - imin) / 2 + imin;
			T val = this->array[imid];
			if (val == value)
				return imid;
			else if (val < value)
			{
				imin = imid + 1;
			}
			else
			{
				imax = imid - 1;
			}
		}

		while (imid < this->n && this->array[imid] < value)
			imid++;

		return -(imid + 1);
	}

	// Szuka w kolekcji elementu, przeszukuj¹c liniowo elementy od pocz¹tku.
	// Zwraca indeks elementu, je¿eli zwrócona wartoœæ jest >= 0.
	// W przeciwnym wypadku zwraca: -(docelowy_indeks + 1).
	DEF int searchBegin(T value) const
	{
		for(int i = 0; i < this->n; i++)
		{
			T val = this->array[i];
			if (value < val)
				return -(i + 1);
			if (val == value)
				return i;
		}

		return -(this->n + 1);
	}

	// Szuka w kolekcji elementu, przeszukuj¹c liniowo elementy od koñca.
	// Zwraca indeks elementu, je¿eli zwrócona wartoœæ jest >= 0.
	// W przeciwnym wypadku zwraca: -(docelowy_indeks + 1).
	DEF int searchEnd(T value) const
	{
		for(int i = this->n - 1; i >= 0; i--)
		{
			T val = this->array[i];
			if (val < value)
				return -(i + 2);
			if (val == value)
				return i;
		}

		return -1;
	}

	// Dodaje element do kolekcji, korzystaj¹c z wyszukiwania binarnego.
	DEF bool add(T value)
	{
		int index = binarySearch(value);
		if (index >= 0)
{
			return false;
}
		index = -index - 1;

		for(int i = this->n; i > index; i--)
		{
			this->array[i] = this->array[i - 1];
		}
		this->array[index] = value;
		this->n++;

		return true;
	}

	// Dodaje element do kolekcji, przeszukuj¹c kolekcjê od pocz¹tku.
	DEF bool addBegin(T value)
	{
		int index = searchBegin(value);
		if (index >= 0)
			return false;
		index = -index - 1;

		for(int i = this->n; i > index; i--)
		{
			this->array[i] = this->array[i - 1];
		}
		this->array[index] = value;
		this->n++;

		return true;
	}

	// Dodaje element do kolekcji, przeszukuj¹c kolekcjê od koñca.
	DEF bool addEnd(T value)
	{
		int index = searchEnd(value);
		if (index >= 0)
			return false;
		index = -index - 1;

		for(int i = this->n; i > index; i--)
		{
			this->array[i] = this->array[i - 1];
		}
		this->array[index] = value;
		this->n++;

		return true;
	}

	DEF bool remove(T value)
	{
		int index = binarySearch(value);
		if (index < 0)
			return false;
		for (int i = index; i < this->n; i++)
		{
			this->array[i] = this->array[i + 1];
		}
		this->n--;
		return true;
	}

	// Sprawdza istnienie elementu, korzystaj¹c z wyszukiwania binarnego.
	DEF bool contains(T value) const
	{
		return binarySearch(value) >= 0;
	}

	// Sprawdza istnienie elementu, przeszukuj¹c kolekcjê od pocz¹tku.
	DEF bool containsBegin(T value) const
	{
		return searchBegin(value) >= 0;
	}

	// Sprawdza istnienie elementu, przeszukuj¹c kolekcjê od koñca.
	DEF bool containsEnd(T value) const
	{
		return searchEnd(value) >= 0;
	}
};

#endif //SORTED_SET_H
