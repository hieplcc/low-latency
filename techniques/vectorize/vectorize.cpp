#include <cstring>
#include <iostream>
#include <chrono>

//Compiler can NOT vectorized the arrays
// since the current iteration depends on the result of previous one.
int a[100000], b[100000], c[100000];

void unvectorizable() {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i <= 99998; ++i) {
        a[i]     += b[i];
        b[i + 1] += c[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "unvectorizable takes: " << duration << " (us)" << std::endl;
}

//Compiler can vectorize the arrays and does SIMD operations
void vectorizable() {
    auto start = std::chrono::high_resolution_clock::now();

    a[1] += b[1]; 
    for (int i = 1; i <= 99998; ++i) {
        b[i + 1] += c[i];
        a[i + 1] += b[i + 1];
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "vectorizable takes: " << duration << " (us)" << std::endl;
}

//Need to compile with -O3 so compiler can optimize and vectorize the input.
int main() {
    unvectorizable();
    vectorizable();
    return 1;
}