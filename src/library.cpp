#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

#include "lock_free_programs.h"

enum program {
    SINGLE_PRODUCER = 0,
    SINGLE_CONSUMER,
    SPLIT_50_50,
    PRODUCE_AND_CONSUME
};

/**
 * Call a certain lock free program @p p
 * @param p         The program to call (0 = single_producer, 1 = single_consumer, 2 = split_50_50)
 * @param t         The amount of threads which will be used for the benchmark
 * @param elements  The amount of elements each producer in the respective benchmark should produce
 * @return Results from the benchmark 
 */
struct benchmark_result small_bench(program p, int t, int elements) {
    switch (p) {
        case SINGLE_PRODUCER:
            return lock_free_programs::single_producer(t, elements);
        case SINGLE_CONSUMER:
            return lock_free_programs::single_consumer(t, elements);
        case SPLIT_50_50:
            return lock_free_programs::split_50_50(t, elements);
        case PRODUCE_AND_CONSUME:
            return lock_free_programs::produce_and_consume(t, elements);
    }
    return {};
}

/** 
 * main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    
    int threads = 8;
    int elements = 10000;
    benchmark_result results[4];
    results[0] = small_bench(SINGLE_PRODUCER, threads, elements);
    results[1] = small_bench(SINGLE_CONSUMER, threads, elements);
    results[2] = small_bench(SPLIT_50_50, threads, elements);
    results[3] = small_bench(PRODUCE_AND_CONSUME, threads, elements);

    for (int i = 0; i < 4; i++) {
        std::cout << "Benchmark: " << i+1 << std::endl
            << "Time: \t   " << results[i].time << " seconds" << std::endl
            << "Results:" << std::endl;
        results[i].reduced_counters.print();

    }
}
