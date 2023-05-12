#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

#include "lock_free_bag.h"

/* These structs should to match the definition in benchmark.py
 */
struct counters {
    int failed_turns;
    int successful_lends;
};
struct bench_result {
    float time;
    struct counters reduced_counters;
};


struct counters random_bench1(int times, int seed) {
    int tid = omp_get_thread_num();
    printf("Thread %d started.\n", tid);
    // Barrier to force OMP to start all threads at the same time
    #pragma omp barrier

    struct counters data = {};
    return data;
}

struct bench_result small_bench(int t) {
    struct bench_result result;
    LockFreeBag bag(t);
    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    small_bench(1);
    small_bench(2);
    small_bench(4);
    small_bench(8);
    small_bench(16);
}
