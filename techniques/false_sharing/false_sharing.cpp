#include <thread>
#include <atomic>
#include <ctime>
#include <chrono>
#include <iostream>

void work(std::atomic<int> &val) {
    for (int i = 0; i < 10000; ++i) {
        val++;
    }
}

void false_sharing() {
    auto start = std::chrono::high_resolution_clock::now();
    std::atomic<int> a;
    std::atomic<int> b;
    std::atomic<int> c;
    std::atomic<int> d;

    std::cout << "All the atomics are in the same cache line: " << std::endl;
    std::cout << "&a: " << &a << std::endl;
    std::cout << "&b: " << &b << std::endl;
    std::cout << "&c: " << &c << std::endl;
    std::cout << "&d: " << &d << std::endl;

    std::thread t1([&](){ work(a); });
    std::thread t2([&](){ work(b); });
    std::thread t3([&](){ work(c); });
    std::thread t4([&](){ work(d); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "false_sharing takes: " << duration << " (us)" <<std::endl;}

struct alignas (64) aligned_type {
    std::atomic<int> val;
};

void avoid_false_sharing() {
    // clock_t start_clock = clock();
    auto start = std::chrono::high_resolution_clock::now();
    aligned_type a;
    aligned_type b;
    aligned_type c;
    aligned_type d;

    std::cout << "The atomics are in the different cache line: " << std::endl;
    std::cout << "&a" << &a << std::endl;
    std::cout << "&b" << &b << std::endl;
    std::cout << "&c" << &c << std::endl;
    std::cout << "&d" << &d << std::endl;

    std::thread t1([&](){ work(a.val); });
    std::thread t2([&](){ work(b.val); });
    std::thread t3([&](){ work(c.val); });
    std::thread t4([&](){ work(d.val); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "avoid_false_sharing takes: " << duration << " (us)" << std::endl;
}


int main() {
    false_sharing();

    std::cout << "=====================================" << std::endl;
    
    avoid_false_sharing();
    return 1;
}