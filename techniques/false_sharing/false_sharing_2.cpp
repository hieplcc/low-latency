#include <thread>
#include <atomic>
#include <ctime>
#include <chrono>
#include <iostream>
#include <mutex>

void work(int &val) {
    for (int i = 0; i < 10000; ++i) {
        val++;
    }
}

void false_sharing() {

    auto start = std::chrono::high_resolution_clock::now();
    int a;
    int b;
    int c;
    int d;

    std::cout << "All the variables are in the same cache line: " << std::endl;
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

void avoid_false_sharing() {
    // clock_t start_clock = clock();
    auto start = std::chrono::high_resolution_clock::now();
    alignas (64) int a;
    alignas (64) int b;
    alignas (64) int c;
    alignas (64) int d;

    std::cout << "The variables are in the different cache line: " << std::endl;
    std::cout << "&a" << &a << std::endl;
    std::cout << "&b" << &b << std::endl;
    std::cout << "&c" << &c << std::endl;
    std::cout << "&d" << &d << std::endl;

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
    std::cout << "avoid_false_sharing takes: " << duration << " (us)" << std::endl;
}


int main() {
    false_sharing();

    std::cout << "=====================================" << std::endl;
    
    avoid_false_sharing();
    return 1;
}