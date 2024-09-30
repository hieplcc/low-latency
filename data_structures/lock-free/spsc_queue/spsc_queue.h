#pragma once

#include <atomic>

template <class T>
class spsc_queue {
public:
    explicit spsc_queue(int capacity) : _capacity(capacity) {
        _capacity++; //note: only _capacity - 1 elements can be stored at a given time
        _slots = new T[_capacity * sizeof(T)];
    }

    template <class... Args>
    void try_emplace(Args&&... args) {
        auto tail = _tail.load(std::memory_order_relaxed); 
        auto next_tail = (tail + 1) % _capacity;
        auto head = _head.load(std::memory_order_acquire);
        if (next_tail == head) // The queue is full
            return false; 

        new (&_slots[tail]) T(std::forward<Args>(args)...); 
        _tail.store(next_tail, std::memory_order_release);
        return true;
    }

    template <class... Args>
    void emplace(Args... args) {
        auto tail = _tail.load(std::memory_order_relaxed); 
        auto next_tail = (tail + 1) % _capacity;
        for (;;) {
            auto head = _head.load(std::memory_order_acquire);
            if (next_tail != head) // The queue is not full
                break;
        }

        new (&_slots[tail]) T(std::forward<Args>(args)...); 
        _tail.store(next_tail, std::memory_order_release);
    }

    T *front() {
        auto head = _head.load(std::memory_order_relaxed);
        auto tail = _tail.load(std::memory_order_acquire);
        if (head == tail) // The queue is empty
            return nullptr;

        return &_slots[head];

    }

    void pop() {
        auto head = _head.load(std::memory_order_relaxed);
        auto tail = _tail.load(std::memory_order_acquire);

        if (head == tail) //The queue is empty
            return false;

        _slots[head].~T();
        auto nextHead = (head + 1) % _capacity;       

        _head.store(nextHead, std::memory_order_release);
    }

private:
    static constexpr auto s_cache_line = std::hardware_destructive_interference_size;
    
    alignas(s_cache_line) std::atomic<size_t> _head;
    alignas(s_cache_line) std::atomic<size_t> _tail;
    alignas(s_cache_line) std::byte* _slots = nullptr;

    //Not change -> no need over-alinged
    int _capacity;
};