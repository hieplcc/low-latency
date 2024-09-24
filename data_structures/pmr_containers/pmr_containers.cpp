#include <list>

#include <memory_resource>
#include <benchmark/benchmark.h>
static void RegularList(benchmark::State& state) {
    for (auto _ : state) {
        std::list<int> l;
        for (int i = 0; i < 100; ++i) {
            l.push_back(i);
        }
    }
}

BENCHMARK(RegularList);

static void PMRList(benchmark::State& state) {
    for (auto _ : state) {
        std::byte buffer[2048];
        std::pmr::monotonic_buffer_resource rs(buffer, sizeof(buffer));
        std::pmr::list<int> l(&rs);
        for (int i = 0; i < 100; ++i) {
            l.push_back(i);
        }
    }
}

BENCHMARK(PMRList);

BENCHMARK_MAIN();