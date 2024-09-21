#include <unordered_map>
#include <map>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <cassert>
#include <benchmark/benchmark.h>

namespace llc {
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

        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; }

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
            if ((_size << 1) > _capacity)
                rehash();

            size_t index;
            bool result = put(key, _nodes, _capacity, index);
            if (result)
                _size++;

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
                    size_t index;
                    (void) put(_nodes[i].key, n_nodes, n_capacity, index);
                    n_nodes[index].value = _nodes[i].value;
                }
            }

            delete[] _nodes;

            _nodes = n_nodes;
            _capacity = n_capacity;
        }

        bool put(const Key& key, node<Key, Value>* nodes, size_t capacity, size_t& index) {
            index = getIndex(key, capacity);

            size_t count = 0;

            while (count < capacity) {
                if (nodes[index].state == node_state::IN_USE 
                        && nodes[index].key == key) {
                    return false;
                } 
                
                if (nodes[index].state == node_state::EMPTY || nodes[index].state == node_state::ERASED) {
                    nodes[index].key = key;
                    nodes[index].state = node_state::IN_USE;
                    return true;
                } 

                index++;
                count++;
                if (index == capacity)
                    index = 0;
            }

            return false;
            throw std::logic_error("Unexpected case!");
        }

        size_t getIndex(const Key& key, size_t size) const {
            return (_hash(key) * 22543) % size;
        }

        size_t _capacity;
        size_t _size = 0;
        Hash _hash;

        node<Key, Value>* _nodes;
    };
}

const int N = 5000000;
int64_t keys[N];

template <typename Container>
void test(const char* name) {
    srand(97);
    for (size_t i = 0; i < N; i++)
        keys[i] = (static_cast<int64_t>(rand()) << 48) ^ (static_cast<int64_t>(rand()) << 32)
            ^ (rand() << 16) ^ rand();

    // clock_t start_clock = clock();

    Container m;

    for (size_t i = 0; i < N; i++)
        m[keys[i]] = keys[i] * keys[i];

    for (size_t i = 0; i < N; i++)
        assert(keys[i] * keys[i] == m[keys[i]]);

    // std::cout << name << " takes " << ((clock() - start_clock) * 1000.0 / CLOCKS_PER_SEC) << std::endl;
}

static void BM_StdMap(benchmark::State& state) {
    for (auto _ : state) {
        test<std::map<int64_t, int64_t>>("std::map");
    }
}
BENCHMARK(BM_StdMap);

static void BM_StdUnorderedMap(benchmark::State& state) {
    for (auto _ : state) {
        test<std::unordered_map<int64_t, int64_t>>("std::unordered_map");
    }
}
BENCHMARK(BM_StdUnorderedMap);

static void BM_LlcDenseHashMap(benchmark::State& state) {
    for (auto _ : state) {
        test<llc::dense_hash_map<int64_t, int64_t>>("llc::dense_hash_map");
    }
}
BENCHMARK(BM_LlcDenseHashMap);

BENCHMARK_MAIN();
