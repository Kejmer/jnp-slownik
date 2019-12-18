#ifndef JNP_INSERTION_ORDERED_MAP
#define JNP_INSERTION_ORDERED_MAP

#include <iostream>
#include <unordered_map>

using namespace std;


/* TODO LIST
Konstruktor bezparametrowy O(1)
Konstruktor kopiujący O(1) O(n) copy-on-write
Konstruktor przenoszący O(1)
Operator przypisania O(1)
Wstawianie do słownika O(1) // smart pointer check
Usuwanie ze słownika O(1) // smart pointer check
Scalanie słowników O(n+m) // iterowanie się po drugim i wstawianie?
Referencja wartości O(1) // smart pointer check
Operator indeksowania O(1)
Rozmiar słownika O(1) – DONE
Sprawdzanie niepustości O(1) – DONE
Czyszczenie słownika O(n) // a nie da się w O(1)?
Sprawdzanie instnienia klucza O(1)
Klasa iteratora: O(1)
	konstruktor kopiujący
	konstruktor bezparametrowy
	operatory:
		++
		==
		!=
		* (dereferencja) 
Klasa lookup_error (na zewnątrz klasy, dziedziczy po std::exception)
Klasa insertion_ordered_map przeźroczysta na wyjątki
*/

/* INNE POMYSŁY 
Jeśli to możliwe sama operacja wstawiania będzie nieudostępnianą operacją
wykorzystywaną przez Wstawianie i Scalanie, aby tuż przed odpaleniem sprawdzać
czy należy kopiować tylko raz.

*/

/* klamrowanie – szybka informacja, do usunięcia
funkcja() 
{
	
}

klasa 
{
	
}

if () {
	
}

for () {
	
}
etc
*/

class lookup_error : exception
{
	
};

template <class K, class V, class Hash = std::hash<K>>
class insertion_ordered_map 
{
private:
	//tutaj ta funkcja do dodawania
	size_t size;
	
	class node 
	{
	public:
		node *next = NULL;
		node *previous;
		V value;
		
		node(V value)
		{
			this->value = value;
			this->previous = NULL; //może wskazywał na siebie?
		}
		
		node(V value, node *previous) //dodawany na końcu
		{
			this->value = value;
			this->previous = previous;
			previous->next = this;
		}
		
	}
	
	//może też specjalna klasa opakowująca mapę?
	// zawiera metodę begin(), end() dla iteratorów
		
		
public:
	insertion_ordered_map() 
	{
		
	}
	
	insertion_ordered_map(insertion_ordered_map const &other) 
	{
		
	}
	
	insertion_ordered_map(insertion_ordered_map &&other) 
	{
		
	}
	
	insertion_ordered_map &operator=(insertion_ordered_map other) 
	{
		
	}
	
	bool insert(K const &k, V const &v) 
	{
		return true;
	}
	
	void erase(K const &k) 
	{
		
	}
	
	void merge(insertion_ordered_map &other)
	{
		
	}
	
	V &at(K const &k)
	{
		return V();
	}
	
	V const &at(K const &k) const 
	{
		return V();
	}
	
	V &operator[](K const &k)
	{
		return V();
	}
	
	size_t size() const 
	{
		return this->size;
	}
	
	bool empty() const
	{
		return this->size == 0;
	}
	
	void clear()
	{
		
	}
	
	bool contains(K const &k) const 
	{
		return true;
	}
	
	class iterator 
	{
		
	};
	
};


#endif //JNP_INSERTION_ORDERED_MAP