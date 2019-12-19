#ifndef JNP_INSERTION_ORDERED_MAP
#define JNP_INSERTION_ORDERED_MAP

#include <iostream>
#include <unordered_map>
#include <memory>

using namespace std;


/* TODO LIST
Konstruktor bezparametrowy O(1) – DONE
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
Czyszczenie słownika O(n) 
Sprawdzanie instnienia klucza O(1) – DONE
Klasa iteratora: O(1)
	konstruktor kopiujący
	konstruktor bezparametrowy
	operatory:
		++
		==
		!=
		* (dereferencja) 
Klasa lookup_error (na zewnątrz klasy, dziedziczy po std::exception) – niepewny czy done
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
	class node 
	{
	public:
		node *next = NULL;
		node *previous;
		bool is_last = false; //dla true powinno zwrócić exception przy dereferencji
		V value;
		
		node() noexcept
		{
			this->is_last = true;
			this->previous = this;
		}
		
		node(V value, node *end) noexcept //konstruktor elementu poczatkowego, wskazuje sam na siebie
		{
			this->value = value;
			this->next = end;
			this->previous = this;
			end->previous = this;
		}
		
		node(V value, node *previous, node *end) noexcept //dodawany na końcu
		{
			this->value = value;
			this->previous = previous;
			this->next = end;
			previous->next = this;
			next->previous = this;
		}
		
	}
	
	class container
	{
		unordered_map<K, node*, Hash> _memory;
				
		size_t size;
		node *begin;
		node *end; //specjalny node który zwraca błąd po dereferencji – do zrobienia
		
		node *last() {
			return end->previous();
		}
		
		container() 
		{
			end = new node();
			begin = end;
			size = 0;
		}
		
		bool insert(K const &k, V const &v) {
			if (begin == end) {
				begin = new node(v, end);
				_memory[k] = begin;
			} else {
				if (_memory.count(k)) { //czasem będziemy robić bezpośrednie inserty (może) przy scalaniu słownika
					return false;
				} else {
					_memory[K] = new node(v, last(), end);
				}
			}
			size++;
			return true;
		}
		
		bool contains(K const &k) {
			return _memory.count(k) != 0;
		}
		
		bool remove(K const &k) {
			auto search = _memory.find(k);
			k->previous->next = k->next;
			k->next->previous = k->previous;
			size--;
			_memory.remove(search);
		}
		
		V &at(K const &k) {
			return _memory.find(k)->value; //niedokońca tak chyba
		}
	}
	shared_ptr<container> _memory_ptr;
	
	//tutaj ta funkcja do dodawania – do zrobienia
	void has_to_copy() {
		throw 
	}
	
	
public:
	class iterator;
	
	insertion_ordered_map() noexcept
	{
		memory_ptr = new shared_ptr<container>(new container());
	}
	
	insertion_ordered_map(insertion_ordered_map const &other) 
	{
		
	}
	
	insertion_ordered_map(insertion_ordered_map &&other) 
	{
		
	}
	
	~insertion_ordered_map()
	{
		// To jest tricky. Jeśli patrzy na wspólny element z kimś innym to wystarczy że przestanie patrzeć na niego 
	}
	
	insertion_ordered_map &operator=(insertion_ordered_map other) 
	{
		
	}
	
	bool insert(K const &k, V const &v) 
	{
		if (this->contains(k)) return false;
		has_to_copy();
		return memory_ptr->insert(k, v); //powinno zawsze true w tym miejscu
	}
	
	void erase(K const &k) 
	{
		if (!this->contains(k)) throw lookup_error();
		has_to_copy();
		_memory_ptr->remove(k);
	}
	
	void merge(insertion_ordered_map &other)
	{
		
	}
	
	V &at(K const &k) //brakuje tej obsłúgi pamięci dzielonej
	{
		return _memory->at(k);
	}
	
	V const &at(K const &k) const 
	{
		return V();
	}
	
	V &operator[](K const &k)
	{
		return this->at(k);
	}
	
	size_t size() const 
	{
		return this->_memory_ptr->size;
	}
	
	bool empty() const
	{
		return size() == 0;
	}
	
	void clear()
	{
		
	}
	
	bool contains(K const &k) const 
	{
		this->_memory_ptr->contains(k);
	}
	
	
private:
	
	iterator create_iterator(node *n) //powinien patrzeć na node a nie wartość, tylko nie powinien
	{
		
	}
	
public:
	
	class iterator //typename?
	{
	insertion_ordered_map &operator=(insertion_ordered_map other) 
	private:
		node *n;
		
	public:
		iterator &operator++()
		{
			return create_iterator(this->n->next) //co jak next to end?
		}
		
		bool operator==(iterator &other)
		{
			return other.n == this->n // nie jestem pewien czy mogę odwołać się do other->n ;friend jakiś może?
		}
		
		bool operator!=(iterator &other)
		{
			return !(*this == other);
		}
		
		V operator*() {
			return n->value;
		}
	};
	
	//czy begin i end w środku czy na końcu?
	
	
	iterator begin() 
	{
		return create_iterator(this->_memory_ptr->begin);
	}
	
	iterator end() 
	{
		return create_iterator(this->_memory_ptr->end);
	}
	
};


#endif //JNP_INSERTION_ORDERED_MAP