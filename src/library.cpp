#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

#include "lock_free_programs.h"
#include "lock_based_programs.h"

extern "C" { // extern C is needed to make the following data available in the shared library

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
struct benchmark_result small_bench(int t, program p, int elements) {
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
 * 
 */
struct lock_benchmark_result small_lock_based_bench(int t, program p, int elements) {
    switch (p) {
        case SINGLE_PRODUCER:
            return lock_based_programs::single_producer(t, elements);
        case SINGLE_CONSUMER:
            return lock_based_programs::single_consumer(t, elements);
        case SPLIT_50_50:
            return lock_based_programs::split_50_50(t, elements);
        case PRODUCE_AND_CONSUME:
            return lock_based_programs::produce_and_consume(t, elements);
    }
    return {};
}

}

/** 
 * main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    
    int threads = 2;
    int elements = 1000;
    // benchmark_result results[4];
    // results[0] = small_bench(threads, SINGLE_PRODUCER, elements);
    // results[1] = small_bench(threads, SINGLE_CONSUMER, elements);
    // results[2] = small_bench(threads, SPLIT_50_50, elements);
    // results[3] = small_bench(threads, PRODUCE_AND_CONSUME, elements);

    // for (int i = 0; i < 4; i++) {
    //     std::cout << "Benchmark: " << i+1 << std::endl
    //         << "Time: \t   " << results[i].time << " seconds" << std::endl
    //         << "Results:" << std::endl;
    //     print_counters(results[i].reduced_counters);

    // }
    lock_benchmark_result result;
    benchmark_result result2;
    // std::cout << "SINGLE_PRODUCER " <<std::endl;
    //result = small_lock_based_bench(threads, SINGLE_PRODUCER, elements);
    //  result2 = small_bench(threads, SINGLE_PRODUCER, elements);
    //std::cout << "Benchmark took " << result.time << " seconds" << std::endl;


    // std::cout << "SINGLE_CONSUMER " <<std::endl;
    // result = small_lock_based_bench(threads, SINGLE_CONSUMER, elements);
    //  result2 = small_bench(threads, SINGLE_CONSUMER, elements);
    ///std::cout << "Benchmark took " << result.time << " seconds" << std::endl;


    std::cout << "SPLIT_50_50 " <<std::endl;
    // result = small_lock_based_bench(threads, SPLIT_50_50, elements);
     result2 = small_bench(threads, SPLIT_50_50, elements);
    ///std::cout << "Benchmark took " << result.time << " seconds" << std::endl;


    // std::cout << "PRODUCE_AND_CONSUME " <<std::endl;
    // result = small_lock_based_bench(threads, PRODUCE_AND_CONSUME, elements);
    //  result2 = small_bench(threads, PRODUCE_AND_CONSUME, elements);
    //std::cout << "Benchmark took " << result.time << " seconds" << std::endl;
}
