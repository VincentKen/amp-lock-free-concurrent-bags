#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

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

struct bench_result small_bench(int t, int len) {
    struct bench_result result;
    struct counters thread_data[t];
    double tic, toc;

    omp_set_num_threads(t);
    tic = omp_get_wtime();
    {
        #pragma omp parallel for
        for (int i=0; i<t; i++) {
            thread_data[i] = random_bench1(len, i);
        }
    }
    toc = omp_get_wtime();

    for (int i=0; i<t; i++) {
        result.reduced_counters.successful_lends += thread_data[i].successful_lends;
        result.reduced_counters.failed_turns     += thread_data[i].failed_turns;
    }

    result.time = (toc - tic);

    return result;
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    small_bench(1, 10);
    small_bench(2, 10);
    small_bench(4, 10);
    small_bench(8, 10);
    small_bench(16, 10);
}
