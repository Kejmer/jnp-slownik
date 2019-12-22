#ifndef JNP_INSERTION_ORDERED_MAP
#define JNP_INSERTION_ORDERED_MAP

#include <iostream>
#include <unordered_map>
#include <memory>
#include <cassert>

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
    std::optional<V> value;
    K key;

    void attach(node *next) noexcept
    {
      this->next = next;
      if (next == next->previous) {
        this->previous = this;
      }
      else {
        this->previous = next->previous;
        this->previous->next = this;
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
    }

    node(const K &key, const V &value, node *next)
    {
      this->value = std::make_optional(value);
      this->key = key;

      attach(next);
    }

    ~node() noexcept
    {
      detach();
    }
  };

  struct container
  {
    node *begin;
    node end;
    std::unordered_map<K, node, Hash> memory;

    container() noexcept
    {
        begin = &end;
    }

    container(const container *other)
    {
      begin = &end;

      node *it = other->begin;
      while (it != &other->end) {
        insert(it->key, it->value.value());
        it = it->next;
      }
    }

    size_t size() const noexcept
    {
      return memory.size();
    }

    bool contains(K const &k) const
    {
        return memory.count(k) != 0;
    }

    void remove(K const &k)
    {
      try {
        if (k == begin->key) begin = begin->next;
        if (memory.erase(k) != 1) throw lookup_error();
      }
      catch (...) {
        begin = begin->previous;
        throw;
      }
    }

    std::pair<node *, bool> insert(K const &k, V const &v)
    {
      auto it = memory.try_emplace(k, k, v, &end);
      if (!it.second) {
        if (k == begin->key) begin = begin->next;
        it.first->second.detach();
        it.first->second.attach(&end);
      }
      begin = begin->previous;

      return std::make_pair(&it.first->second,it.second);
    }

    V &at(K const &k)
    {
      if (memory.count(k) != 1) throw lookup_error();
      return memory[k].value.value();
    }
  };

  std::shared_ptr<container> memory_ptr;
// Kiedy tworzymy referencję na obiekt w mapie nie mamy pewności czy ktoś
// tego obiektu nie zmieni.
  bool exists_reference = false;

  void has_to_copy()
  {
    if (memory_ptr.use_count() > 1) {
      copy_on_write();
    }
  }

  void copy_on_write()
  {
    memory_ptr = std::make_shared<container>(memory_ptr.get());
  }

public:
  class iterator;

  insertion_ordered_map()
  {
    memory_ptr = std::make_shared<container>();
  }

  insertion_ordered_map(insertion_ordered_map const &other)
  {
    // Trzeba sprawdzić czy nie istnieją referencje na elementy container.
    memory_ptr = other.memory_ptr;
    if (other.exists_reference) {
      copy_on_write();
    }
  }

  insertion_ordered_map(insertion_ordered_map &&other) noexcept
  {
    exists_reference = other.exists_reference;
    memory_ptr = other.memory_ptr;
  }

  insertion_ordered_map &operator=(insertion_ordered_map other)
  {
    if (memory_ptr.get() == other.memory_ptr.get()) return *this;

    std::shared_ptr<container> old_memory_ptr = memory_ptr;
    try {
      memory_ptr = other.memory_ptr;
      if (other.exists_reference) {
          copy_on_write();
      }
      exists_reference = false;

      return *this;
    }
    catch(...) {
      memory_ptr = old_memory_ptr;
      throw;
    }
  }

  bool insert(K const &k, V const &v)
  {
    has_to_copy();
    return memory_ptr->insert(k, v).second;
  }

  void erase(K const &k)
  {
    has_to_copy();
    memory_ptr->remove(k);
  }

  void merge(insertion_ordered_map const &other)
  {
    // Merge siebie ze sobą nic nie da.
    if (memory_ptr.get() == other.memory_ptr.get()) return;

    std::shared_ptr<container> old_memory_ptr = memory_ptr;
    try {
      has_to_copy();
      iterator it = other.begin();
      iterator fin = other.end();
      while (it!=fin) {
        memory_ptr->insert(it->first, it->second);
        ++it;
      }
    }
    catch (...) {
      memory_ptr = old_memory_ptr;
      throw;
    }
  }

  V &at(K const &k)
  {
    has_to_copy();
    exists_reference = true;
    return memory_ptr->at(k);
  }

  V const &at(K const &k) const
  {
    return memory_ptr->at(k);
  }

  template <typename = std::enable_if_t<std::is_default_constructible<V>::value>>
  V &operator[](K const &k)
  {
    has_to_copy();
    exists_reference = true;
    if (memory_ptr->contains(k))
      return memory_ptr->at(k);
    return memory_ptr->insert(k, V()).first->value.value();
  }

  size_t size() const noexcept
  {
    return memory_ptr->size();
  }

  bool empty() const noexcept
  {
    return size() == 0;
  }

  // Po prostu przestajemy patrzeć.
  void clear()
  {
    memory_ptr = std::make_shared<container>();
  }

  bool contains(K const &k) const
  {
    return memory_ptr->contains(k);
  }

private:
  iterator create_iterator(node *n) const
  {
    iterator it;
    it.n = n;
    it.stored_pair = n->next == nullptr ?
            std::optional<std::pair<K, V>>() :
            std::make_optional(std::make_pair(n->key, n->value.value()));
    return it;
  }

public:
  class iterator
  {
  private:
    insertion_ordered_map &operator=(insertion_ordered_map other);

    node *n;
    std::optional<std::pair<K,V>> stored_pair;

    friend class insertion_ordered_map;

  public:
    iterator &operator++()
    {
      n = n->next;
      if (n == nullptr)
        throw lookup_error();

      try {
        if (n->next != nullptr) {
          stored_pair =
                  std::make_optional(std::make_pair(n->key, n->value.value()));
        }
      }
      catch (...) {
        n = n->previous;
        throw;
      }

      return *this;
    }

    bool operator==(iterator &other) const noexcept
    {
      return other.n == n;
    }

    bool operator!=(iterator &other) const noexcept
    {
      return !(*this == other);
    }

    const std::pair<K,V> &operator*() const {
      if (n->next == nullptr) throw lookup_error();
      return stored_pair.value();
    }
    const std::pair<K,V> *operator->() const {
      if (n->next == nullptr) throw lookup_error();
      return &stored_pair.value();
    }
  };

  iterator begin() const noexcept
  {
    return create_iterator(memory_ptr->begin);
  }

  iterator end() const noexcept
  {
    return create_iterator(&memory_ptr->end);
  }

};


#endif //JNP_INSERTION_ORDERED_MAP