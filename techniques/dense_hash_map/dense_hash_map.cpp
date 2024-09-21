#include <unordered_map>
#include <stdexcept>

enum class node_state {
    EMPTY,
    IN_USE,
    ERASED
};

template <class Key, class Value>
struct node {
    node_state state = node_state::EMPTY;
    Key key;
    Value value;
};

template <class Key, class Value, class Hash = std::hash<Key>>
class dense_hash_map {
public:
    dense_hash_map(size_t capacity) : _capacity(capacity) {
        _nodes = new node<Key, Value>[_capacity];
        
        for (size_t i = 0; i < _capacity; ++i) {
            _nodes[i] = node<Key, Value>();
        }
    }

    dense_hash_map() : dense_hash_map(3) {}

    bool find(const Key& key) const {
        size_t index = getIndex(key, _capacity);

        size_t count = 0;
        while (count < _capacity) {
            if (_nodes[index].state == node_state::EMPTY)
                return false;

            if (_nodes[index].state == node_state::IN_USE
                    && _nodes[index].key == key)
                return true;

            index++;
            count++;
            if (index == _capacity)
                index = 0;
        }

        return false;
    }

    Value& operator[](const Key& key) {
        size_t index = put(key, _nodes, _capacity);
        return _nodes[index].value;
    }

private:
    void rehash() {
        size_t n_capacity = _capacity << 1;
        node<Key, Value>* n_nodes = new node<Key, Value>[n_capacity];
        for (size_t i = 0; i < n_capacity; i++)
            n_nodes[i] = node<Key, Value>();

        for (size_t i = 0; i < _capacity; ++i) {
            if (_nodes[i].state == node_state::IN_USE) {
                size_t index = put(_nodes[i].key, n_nodes, n_capacity);
                n_nodes[index].value = _nodes[i].value;
            }
        }

        delete[] _nodes;
        _nodes = n_nodes;
        _capacity = n_capacity;
    }

    size_t put(const Key& key, node<Key, Value>* nodes, size_t capacity) {
        size_t index = getIndex(key, capacity);

        size_t count = 0;
        while (count < capacity) {
            if (_nodes[index].state == node_state::IN_USE 
                    && _nodes[index].key == key) {
                return index;
            } 
            
            if (_nodes[index].state == node_state::EMPTY || _nodes[index].state == node_state::ERASED) {
                _size++;
                if ((_size << 1) > capacity)
                    rehash();
                
                _nodes[index].key = key;
                _nodes[index].state = node_state::IN_USE;
                return index;
            } 

            index++;
            count++;
            if (index == capacity)
                index = 0;
        }

        throw std::logic_error("Unexpected case!");
    }

    size_t getIndex(const Key& key, size_t size) {
        return (_hash(key) * 22543) % size;
    }

    size_t _capacity;
    size_t _size;
    Hash _hash;

    node<Key, Value>* _nodes;
};

int main() {
    return 1;
}