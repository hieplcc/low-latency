#include <benchmark/benchmark.h>
#include <cstring>
#include <iostream>

//Compiler can NOT vectorized the arrays
// since the current iteration depends on the result of previous one.
static void Unvectorizable(benchmark::State& state) {
    for (auto _ : state) {
        int a[1000], b[1000], c[1000];

        for (int i = 0; i <= 998; ++i) {
            a[i]     += b[i];
            b[i + 1] += c[i];
        }
    }
}
BENCHMARK(Unvectorizable);

//Compiler can vectorize the arrays and does SIMD operations
static void Vectorizable(benchmark::State& state) {
    for (auto _ : state) {
        int a[1000], b[1000], c[1000];
        a[1] += b[1]; 
        for (int i = 1; i <= 998; ++i) {
            b[i + 1] += c[i];
            a[i + 1] += b[i + 1];
        }
    }
}
BENCHMARK(Vectorizable);

// https://quick-bench.com

BENCHMARK_MAIN();

