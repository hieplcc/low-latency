#include <iostream>
#include <unordered_map>
#include <bits/stdc++.h>
#include <benchmark/benchmark.h>

using namespace std;

struct Chunk {
    Chunk *next;
};

class MemoryPool {
public:
    MemoryPool(size_t numChunksPerBlock) : _numChunksPerBlock(numChunksPerBlock) {}

    void* allocate(size_t objectSize) {
        if (_freeChunk == nullptr) {
            _freeChunk = allocateBlock(objectSize);
        }

        Chunk* dumm = _freeChunk;
        _freeChunk = _freeChunk->next;
        return dumm;
    }

    void deallocate(void* ptr, size_t objectSize) {
        Chunk* temp = reinterpret_cast<Chunk*>(ptr);
        temp->next = _freeChunk;
        _freeChunk = temp;
    }

private:
    Chunk* allocateBlock(size_t objectSize) {
        cout << "\nAllocating block (" << _numChunksPerBlock << " chunks):\n\n";
        size_t chunkSize = std::max(objectSize, sizeof(Chunk));

        Chunk* head = reinterpret_cast<Chunk*>(malloc(chunkSize * _numChunksPerBlock));
        Chunk* dumm = head;
        for (int i = 0; i < _numChunksPerBlock - 1; ++i) {
            dumm->next = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(dumm) + chunkSize);
            dumm = dumm->next;
        }

        dumm->next = nullptr;
        return head;
    }

    Chunk* _freeChunk = nullptr;
    size_t _numChunksPerBlock;
};

struct PoolObject {
    // Object data, 16 bytes:
    
    uint32_t data[1];
    
    // Declare out custom allocator for
    // the `Object` structure:
    
    static MemoryPool allocator;
    
    static void *operator new(size_t objectSize) {
        return allocator.allocate(objectSize);
    }
    
    static void operator delete(void *ptr, size_t objectSize) {
        return allocator.deallocate(ptr, objectSize);
    }
};

struct NormalObject {
    // Object data, 16 bytes:
    
    uint32_t data[1];
};
 
// Instantiate our allocator, using 8 chunks per block:
 
MemoryPool PoolObject::allocator{1000};

// int main() {
//     constexpr int arraySize = 10;
 
//     PoolObject *objects[arraySize];
    
//     // Two `uint64_t`, 16 bytes.
//     cout << "size(Object) = " << sizeof(Object) << endl << endl;
    
//     // Allocate 10 objects. This causes allocating two larger,
//     // blocks since we store only 8 chunks per block:
    
//     cout << "About to allocate " << arraySize << " objects" << endl;
    
//     for (int i = 0; i < arraySize; ++i) {
//         objects[i] = new PoolObject();
//         cout << "new [" << i << "] = " << objects[i] << endl;
//     }
    
//     cout << endl;
    
//     // Deallocated all the objects:
    
//     for (int i = arraySize - 1; i >= 0; --i) {
//         cout << "delete [" << i << "] = " << objects[i] << endl;
//         delete objects[i];
//     }
    
//     cout << endl;
    
//     // New object reuses previous block:
    
//     objects[0] = new PoolObject();
//     cout << "new [0] = " << objects[0] << endl << endl;
//     return 1;
// }


static void BM_NormalObject(benchmark::State& state) {
    for (auto _ : state) {
        NormalObject* object = new NormalObject();
        delete object;
    }
}
BENCHMARK(BM_NormalObject);

static void BM_PoolObject(benchmark::State& state) {
    for (auto _ : state) {
        PoolObject* object = new PoolObject();
        delete object;    }
}
BENCHMARK(BM_PoolObject);

BENCHMARK_MAIN();

