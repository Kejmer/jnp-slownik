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

    void attach(node *next) noexcept
    {
      this->next = next;
      if (next == next->previous) {
        this->previous = this;
      }
      else {
        previous = next->previous;
        previous->next = this;
      }
      next->previous = this;
    }

    void detach() noexcept
    {
      if (next == nullptr) return;
      previous->next = next;
      next->previous = previous;
    }

// Element jest ostatni, gdy next == nullptr, a pierwszy, gdy previous == this.
		node() noexcept
		{
			this->previous = this;

			// FIXME: Nie ma V()!
		}

		node(const K &key, const V &value, node *next) //doklejamy tuż przed next
		{
			this->value = value;
			this->key = key;

      attach(next);
		}

		~node() {
      detach();
		}
//		node(const K &key, const V &value, node *previous, node *end) //dodawany na końcu
//		{
//			this->value = value;
//			this->previous = previous;
//			this->next = end;
//			this->key = key;
//            previous->next = this;
//            next->previous = this;
//		}

	};

	class container
	{
	public:
        node *begin;
        node end; // end MUSI być przed _memory !
		std::unordered_map<K, node, Hash> _memory;

		node &last() // FIXME: czy to potrzebne?
		{
			return *end->previous;
		}

		container()
		{
	    end = node();
			begin = &end;
		}

    container(container *other) {
      end = node();
      begin = &end; //to samo co na górze, skompresować potem

      node *it = other.begin;
      while (it != nullptr) {
        this->insert(it->key, it->value);
        it = it->next;
      }
    }

    size_t size() {
      return _memory.size();
    }

    bool contains(K const &k)
    {
        return _memory.count(k) != 0;
    }

    bool remove(K const &k)
    {
        return _memory.erase(k) != 1;
    }

		bool insert(K const &k, V const &v)
		{
	    auto it = _memory.try_emplace(k, k, v, &end);
	    if (!it.second) {
        it.first->detach();
        it.first->attach(&end);
	    }
		    // Co z begin?

			if (begin == &end) {
				_memory.emplace(); // TODO
			} else {
				if (_memory.count(k)) { //czasem będziemy robić bezpośrednie inserty (może) przy scalaniu słownika
					return false;
				} else {
					//_memory[K] = new node(k, v, last(), end);
				}
			}
			return true;
		}

		V &at(K const &k)
		{
			return _memory[k].value;
		}

		// void clean() {
		// 	node *temp;
		// 	while (begin != &end) {
		// 		temp = begin;
		// 		begin = begin->next;
		// 		delete temp;
		// 	}
		// 	// delete end;
		// }

		// ~container()
		// {
		// 	clean();
		// 	//mapa się chyba sama usuwa?
		// }
	};

	std::shared_ptr<container> memory_ptr;

	void has_to_copy() {
    if (memory_ptr.use_count() > 1) {
      copy_on_write();
    }
	}
  bool exists_reference = false; //kiedy tworzymy referencję na obiekt w mapie nie mamy pewności czy ktoś tego obiektu nie zmieni

  void copy_on_write() {
    memory_ptr.reset(new container(memory_ptr.get()));
  }


public:
	class iterator;

	insertion_ordered_map() noexcept
	{
		memory_ptr = std::shared_ptr<container>(new container());
	}

	insertion_ordered_map(insertion_ordered_map const &other)
	{
    //trzeba sprawdzić czy nie istnieją referencje na elementy container
    memory_ptr = std::shared_ptr(other.memory_ptr);
    if (other.exists_reference) {
      copy_on_write();
    }
	}

	insertion_ordered_map(insertion_ordered_map &&other)
	{
    memory_ptr = std::shared_ptr(other.memory_ptr);
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

	V &at(K const &k)
	{
    exists_reference = true;
		return this->memory_ptr->at(k);
	}

	V const &at(K const &k) const
	{
		return this->memory_ptr->at(k);
	}

	V &operator[](K const &k) //czy tutaj się liczy jak referencja? do sprawdzenia
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
		return this->memory_ptr->contains(k);
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
			return other.n == this->n;
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