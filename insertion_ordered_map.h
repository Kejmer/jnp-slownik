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
          if (previous == this) {
            next->previous = next;
            return;
          }
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

		~node()
		{
          detach();
		}
	};

	struct container
	{
        node *begin;
        node end; // end MUSI być przed _memory !
		std::unordered_map<K, node, Hash> _memory;

		container() noexcept
		{
		    end = node();
			begin = &end;
		}

    container(const container *other)
    {
      end = node();
      begin = &end; //skompresować potem

      node *it = other->begin;
      while (it != &other->end) { this->insert(it->key, it->value);
        it = it->next;
      }
    }

    size_t size() const noexcept
    {
      return _memory.size();
    }

    bool contains(K const &k) const //bez noexcpt bo count nie jest
    {
        return _memory.count(k) != 0;
    }

    bool remove(K const &k)
    {
      try {
        if (k == begin->key) begin = begin->next;
        return _memory.erase(k) != 1;
      }
      catch (...) {
        begin = begin->previous;
        throw;
      }
    }

		bool insert(K const &k, V const &v)
		{
          auto it = _memory.try_emplace(k, k, v, &end);
          if (!it.second) {
              if (k == begin->key) begin = begin->next;
              it.first->second.detach();
              it.first->second.attach(&end);
          }
          begin = begin->previous;

          return it.second;
		}

		node &find(K const &k)
        {
          return _memory[k];
        }

		V &at(K const &k)
		{
          return _memory[k].value;
		}

		void clear() noexcept
        {
          _memory.clear();
          begin = &end;
        }
	};

	std::shared_ptr<container> memory_ptr;

	void has_to_copy()
  {
    if (memory_ptr.use_count() > 1) {
      copy_on_write();
    }
	}
  bool exists_reference = false; //kiedy tworzymy referencję na obiekt w mapie nie mamy pewności czy ktoś tego obiektu nie zmieni

  void copy_on_write()
  {
    memory_ptr.reset(new container(memory_ptr.get()));
  }


public:
	class iterator;

	insertion_ordered_map()
	{
		memory_ptr = std::make_shared<container>();
	}

	insertion_ordered_map(insertion_ordered_map const &other)
	{
    //trzeba sprawdzić czy nie istnieją referencje na elementy container
    memory_ptr = std::shared_ptr<container>(other.memory_ptr);
    if (other.exists_reference) {
      copy_on_write();
    }
	}

	insertion_ordered_map(insertion_ordered_map &&other) noexcept
	{
    memory_ptr = std::shared_ptr(other.memory_ptr);
	}

	~insertion_ordered_map() noexcept
	{
		memory_ptr.reset();
	}

	insertion_ordered_map &operator=(insertion_ordered_map other)
	{
    exists_reference = false;
    memory_ptr = std::shared_ptr(other.memory_ptr);
    if (other.exists_reference) {
      copy_on_write();
    }

    return *this;
	}

	bool insert(K const &k, V const &v) noexcept
	{
		if (this->contains(k)) return false;
		has_to_copy();
		return memory_ptr->insert(k, v); // assert true
	}

	void erase(K const &k)
	{
		if (!this->contains(k)) throw lookup_error();
		has_to_copy();
		memory_ptr->remove(k);
	}

	void merge(insertion_ordered_map &other)
	{
    if (memory_ptr == other.memory_ptr) return; //merge siebie ze sobą nic nie da
		has_to_copy();
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

  template <typename = std::enable_if_t<std::is_default_constructible<V>::value>>
	V &operator[](K const &k) //czy tutaj się liczy jak referencja? do sprawdzenia
	{
    if (!this->contains(k))
      this->insert(k, V());
		return this->at(k);
	}

	size_t size() const noexcept
	{
		return this->memory_ptr->size;
	}

	bool empty() const noexcept
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

	iterator create_iterator(node *n) const
	{
    iterator it;
    it.n = n;
    return it;
	}

public:

	class iterator
	{
	insertion_ordered_map &operator=(insertion_ordered_map other);
	private:
		node *n;

		friend class insertion_ordered_map;
	public:
		iterator operator++() //bez noexcept bo nullptr
		{
			this->n = this->n->next; //co jak next to end?
      return *this;
		}

		bool operator==(iterator &other) const noexcept
		{
			return other.n == this->n;
		}

		bool operator!=(iterator &other) const noexcept
		{
			return !(*this == other);
		}

		const V &operator*() const {
			return n->value;
		}
	};

	iterator begin() const noexcept
	{
		return create_iterator(this->memory_ptr->begin); //przerobić obie funkcje aby nie tworzyły za każdym razem tylko ref zwracały – pamiętać o copy on write
	}

	iterator end() const noexcept
	{
		return create_iterator(&this->memory_ptr->end);
	}

};


#endif //JNP_INSERTION_ORDERED_MAP