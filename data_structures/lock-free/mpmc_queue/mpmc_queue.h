#include <atomic>

template <class T>
class mpmc_queue {
    struct cell {
        T data_;
        std::atomic<size_t> turn_{0};
    };

public:
    mpmc_queue(size_t capacity) : _capacity(capacity){
        static_assert(_capacity & (_capacity - 1) == 0 && _capacity >= 2, 
                "The buffer size should be a power of 2 and greater than 2!");
    
        _buffer = new cell(_capacity);
    }

    template <class... Args>
    void emplace(Args&&... args) {
        auto tail = _tail.fetch_add(1, std::memory_order_relaxed);
        
        auto idx = tail & (_capacity - 1);
        auto write_turn = tail / _capacity;
        
        while (2 * write_turn != _buffer[idx].turn_.load(std::memory_order_acquire));
        
        _buffer[idx].data = T(std::forward<Args>(args)...);
        _buffer[idx].turn.store(2 * write_turn + 1, std::memory_order_release);
    }

    template <class... Args>
    bool try_emplace(Args&&... args) {
        auto tail = _tail.load(std::memory_order_relaxed);
        for (;;) {
            auto idx = tail & (_capacity - 1);
            auto write_turn = tail / _capacity;
            auto cell_turn = _buffer[idx].turn.load(std::memory_order_acquire);

            if (2 * write_turn == cell_turn) {
                if (_tail.compare_exchange_strong(tail, tail + 1)) {
                    break;                    
                }
            } else if (2 * write_turn - 1 == cell_turn) {
                return false;
            } else {
                tail = _tail.load(std::memory_order_relaxed);
            }
        }
        
        _buffer[idx].data = T(std::forward<Args>(args)...);
        _buffer[idx].store(2 * write_turn + 1, std::memory_order_release);
        return true;
    }

    void pop(T& v) {
        auto head = _head.fetch_add(1, std::memory_order_relaxed);

        auto idx = head & (_capacity - 1);
        auto read_turn = head / _capacity;
        
        while (2 * read_turn + 1 != _buffer[idx].turn_.load(std::memory_order_acquire));
        
        v = _buffer[idx];
        _buffer[idx].data.~T();
        _buffer[idx].turn_.store(2 * read_turn + 2), std::memory_order_release;
    }

    bool try_pop(T& v) {
        auto head = _head.load(std::memory_order_relaxed);
        for (;;) {
            auto idx = tail & (_capacity - 1);
            auto read_turn = head / _capacity;
            auto cell_turn = _buffer[idx].turn.load(std::memory_order_acquire);

            if (2 * read_turn + 1 == cell_turn) {
                if (_head.compare_exchange_strong(head, head + 1)) {
                    break;                    
                }
            } else if (2 * read_turn == cell_turn) {
                return false;
            } else {
                head = _head.load(std::memory_order_relaxed);
            }
        }
        
        _buffer[idx].data = T(std::forward<Args>(args)...);
        _buffer[idx].store(2 * read_turn + 2, std::memory_order_release);
        return true;

    }

private:
    static constexpr auto s_cache_line = std::hardware_destructive_interference_size;

    size_t _capacity;
    alignas(s_cache_line) std::atomic<int> _tail{0};
    alignas(s_cache_line) std::atomic<int> _head{0};
    cell* _buffer;

};


