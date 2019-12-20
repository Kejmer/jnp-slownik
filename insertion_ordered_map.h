#ifndef JNP_INSERTION_ORDERED_MAP
#define JNP_INSERTION_ORDERED_MAP

#include <iostream>
#include <unordered_map>
#include <memory>

/* TODO LIST
Konstruktor bezparametrowy O(1) – DONE
Konstruktor kopiujący O(1) O(n) copy-on-write
Konstruktor przenoszący O(1)
Operator przypisania O(1)
Wstawianie do słownika O(1) – DONE
Usuwanie ze słownika O(1) – DONE
Scalanie słowników O(n+m) – prawie DONE
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

class lookup_error : std::exception
{
	
};

template <class K, class V, class Hash = std::hash<K>>
class insertion_ordered_map 
{
private:
	struct node
	{
		node *next = nullptr;
		node *previous;
		V value;
		K key;
		
		node() noexcept
		{
			this->previous = this;
			// FIXME: Nie ma V()!
		}
		
		node(const K &key, const V &value, node *end) //konstruktor elementu poczatkowego, wskazuje sam na siebie
		{
			this->value = value;
			this->next = end;
			this->previous = this;
			this->key = key;
            end->previous = this;
		}
		
		node(const K &key, const V &value, node *previous, node *end) //dodawany na końcu
		{
			this->value = value;
			this->previous = previous;
			this->next = end;
			this->key = key;
            previous->next = this;
            next->previous = this;
		}
		
	};
	
	class container
	{
		std::unordered_map<K, node, Hash> _memory;

		node &begin;
		node end;
		
		node &last()
		{
			return *end->previous;
		}
		
		container() 
		{
		    end = node();
			begin = end;
		}
		
		bool insert(K const &k, V const &v) 
		{
			if (begin == end) {
				begin = new node(k, v, end);
				_memory[k] = begin;
			} else {
				if (_memory.count(k)) { //czasem będziemy robić bezpośrednie inserty (może) przy scalaniu słownika
					return false;
				} else {
					//_memory[K] = new node(k, v, last(), end);
				}
			}
			return true;
		}
		
		bool contains(K const &k) 
		{
			return _memory.count(k) != 0;
		}
		
		bool remove(K const &k) 
		{
			auto search = _memory.find(k);
			k->previous->next = k->next;
			k->next->previous = k->previous;
			_memory.remove(search);
		}
		
		V &at(K const &k)
		{
			return _memory.find(k)->value; //niedokońca tak chyba
		}
		
		void clean() {
			node *temp;
			while (begin != end) {
				temp = begin;
				begin = begin->next;
				delete temp;
			}
			delete end;
		}
		
		~container()
		{
			clean();
			//mapa się chyba sama usuwa?
		}
	};
	std::shared_ptr<container> memory_ptr;

	//tutaj ta funkcja do dodawania – do zrobienia
	void has_to_copy() {
		throw;
	}
	
	
public:
	class iterator;

	insertion_ordered_map() noexcept
	{
		memory_ptr = shared_ptr<container>(new container());
	}

	insertion_ordered_map(insertion_ordered_map const &other)
	{

	}

	insertion_ordered_map(insertion_ordered_map &&other)
	{

	}

	~insertion_ordered_map()
	{
		memory_ptr.reset();
	}

	insertion_ordered_map &operator=(insertion_ordered_map other)
	{

	}

	bool insert(K const &k, V const &v) noexcept
	{
		if (this->contains(k)) return false;
		has_to_copy();
		return memory_ptr->insert(k, v); //powinno zawsze true w tym miejscu
	}

	void erase(K const &k)
	{
		if (!this->contains(k)) throw lookup_error();
		has_to_copy();
		memory_ptr->remove(k);
	}

	void merge(insertion_ordered_map &other)
	{
		has_to_copy(); // unless kopiuje siebie w siebie, wtedy nic się nie zmieni
		iterator it = other.begin();
		iterator fin = other.end();
		while (it != fin) {
			memory_ptr->insert(it.n->key, it.n->value);
			++it;
		}
	}

	V &at(K const &k) //brakuje tej obsłúgi pamięci dzielonej
	{
		return this->memory_ptr->at(k);
	}

	V const &at(K const &k) const //tutaj nie potrzeba obsługi pamięci
	{
		return this->memory_ptr->at(k);
	}

	V &operator[](K const &k)
	{
		return this->at(k);
	}

	size_t size() const
	{
		return this->memory_ptr->size;
	}

	bool empty() const
	{
		return this->size() == 0;
	}

	void clear() //po prostu przestajemy patrzeć
	{
		this->memory_ptr.reset(new container());
	}

	bool contains(K const &k) const
	{
		this->memory_ptr->contains(k);
	}


private:

	iterator create_iterator(node *n)
	{

	}

public:

	class iterator
	{
	insertion_ordered_map &operator=(insertion_ordered_map other);
	private:
		node *n;

	public:
		iterator &operator++()
		{
			return create_iterator(this->n->next); //co jak next to end?
		}

		bool operator==(iterator &other)
		{
			return other.n == this->n
		}

		bool operator!=(iterator &other)
		{
			return !(*this == other);
		}

		V operator*() {
			return n->value;
		}

		friend class insertion_ordered_map;
	};

	//czy begin i end w środku czy na końcu?


	iterator begin()
	{
		return create_iterator(this->memory_ptr->begin); //przerobić obie funkcje aby nie tworzyły za każdym razem tylko ref zwracały – pamiętać o copy on write
	}

	iterator end()
	{
		return create_iterator(this->memory_ptr->end);
	}
	
};


#endif //JNP_INSERTION_ORDERED_MAP