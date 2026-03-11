# Workshop: Parallel Programming with pthreads and OpenMP

Examples for the parallel programming workshop covering pthreads and OpenMP.

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Examples

### Counter (`counter/`)

Demonstrates thread-safe counter increment with various synchronization mechanisms:

- **no_sync** - without synchronization (demonstrates race condition)
- **no_sync_test** - test showing race condition
- **mutex** - mutex-based synchronization
- **spinlock** - spinlock-based synchronization  
- **atomic** - C11 atomics
- **mutex_opt** - optimized mutex version
- **atomic_opt** - optimized atomics
- **omp_no_sync** - OpenMP without synchronization
- **omp** - OpenMP with critical sections

Run: `./build/counter/counter_<example>`

### Philosophers (`philosophers/`)

Classic dining philosophers problem with pthreads.

Run: `./build/philosophers/pthread_philosophers`

### Synchronization (`synchronization/`)

Basic pthread synchronization primitives demonstration.

Run: `./build/synchronization/synchronization`

### OpenMP (`openmp/`)

OpenMP parallel programming examples.

Run: `./build/openmp/omp_multiplication`

## Requirements

- CMake 3.17+
- GCC with pthread and OpenMP support
