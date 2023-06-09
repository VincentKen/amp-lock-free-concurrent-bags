#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "lock_free_bag.h"

class lock_free_programs
{
private:
    /* data */
public:

    /**
     * 50% of threads will be produces and 50% will be consumers
     * Each producer only produces 1 item
     */
    static benchmark_result split_50_50_single(int threads) {
        return split_50_50(threads, 1);
    }

    /**
     * One of the threads will be designated as producer and all the others will be consumers
     * The producer will produce @p elements once all these elements have been consumed the benchmark is over
     * @param threads   The number of threads (must be >= 2)
     * @param elements  The number of elements to be produced by the producer 
     */
    static benchmark_result single_producer(int threads, int elements) {
        benchmark_result results;
        if (threads < 2) return results;
        LockFreeBag bag(threads);
        #ifdef DEBUG
            std::cout << "Single producer with " << threads << " threads and " << elements << " elements" << std::endl << std::endl;
        #endif
        std::atomic_int consumed = 0;
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id == 0) { // code ran by producer
                for (int e = 0; e < elements; e++) {
                    bag.Add(id, e);
                }
            } else { // code ran by the consumer threads
                while (consumed < elements) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        consumed++;
                    }
                }
            }
        }
        results.time = omp_get_wtime() - t;
        for (int i = 0; i < threads; i++) {
            add_results(&results, bag.GetCounters(i));
            #ifdef DEBUG
                std::cout << "Results from Thread: " << i << std::endl;
                print_counters(bag.GetCounters(i));
            #endif
        }
        return results;
    }

    /**
     * One of the threads will be designated as consumer and all the others will be producers
     * The @p elements will be evenly divided over each producer.
     * Once all these elements have been consumed the benchmark is over
     * @param threads   The number of threads (must be >= 2)
     * @param elements  The number of elements to be produced
     */
    static benchmark_result single_consumer(int threads, int elements) {
        benchmark_result results;
        if (threads < 2) return results;
        LockFreeBag bag(threads);

        int consumed = 0;
        int e_per_p = elements/(threads - 1); // the amount of elements each producer needs to produce
        elements = e_per_p*(threads - 1); // the new total amount of elements which will be produced, incase previous amount was not evenly divisable
        #ifdef DEBUG
            std::cout << "Single consumer with " << threads << " threads and " << elements << " elements" << std::endl;
            std::cout << "Each thread will produce " << e_per_p << " elements" << std::endl << std::endl;
        #endif
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id == 0) { // code ran by consumer
                while (consumed < elements) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        consumed++;
                    }
                }
                
            } else { // code ran by the producer threads
                for (int e = 0; e < e_per_p; e++) {
                    bag.Add(id, e);
                }
            }
        }
        results.time = omp_get_wtime() - t;
        for (int i = 0; i < threads; i++) {
            add_results(&results, bag.GetCounters(i));
            #ifdef DEBUG
                std::cout << "Results from Thread: " << i << std::endl;
                print_counters(bag.GetCounters(i));
            #endif
        }
        return results;
    }

    /**
     * 50% of threads will be produces and 50% will be consumers
     * The @p elements will be evenly divided between the producers
     * The benchmark is over once all elements have been consumed
     */
    static benchmark_result split_50_50(int threads, int elements) {
        benchmark_result results;
        if (threads % 2 != 0) threads--; // make sure there is an even amount of threads
        if (threads < 2) return results;

        LockFreeBag bag(threads);
        std::atomic_int consumed = 0;
        int e_per_p = elements/(threads/2); // amount of elements each producer has to produce
        elements = e_per_p*(threads/2); // new total amount incase the previous amount was not evenly divisable between the producers
        #ifdef DEBUG
            std::cout << "Split 50 50 with " << threads << " threads and " << elements << " elements" << std::endl;
            std::cout << "Each thread will produce " << e_per_p << " elements" << std::endl << std::endl;
        #endif
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id % 2 == 0) { // produce
                for (int e = 0; e < e_per_p; e++) {
                    bag.Add(id, e);
                }
            } else { // consume
                while (consumed < elements) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        consumed++;
                    }
                }
            }
        }
        results.time = omp_get_wtime() - t;

        for (int i = 0; i < threads; i++) {
            add_results(&results, bag.GetCounters(i));
            #ifdef DEBUG
                // std::cout << "Results from Thread: " << i << std::endl;
                // print_counters(bag.GetCounters(i));
            #endif
        }
        #ifdef DEBUG
        std::cout << "Benchmark ran in " << results.time << " seconds" << std::endl;
        #endif
        return results;
    }

    /**
     * All threads will first produce all of their items and then try to remove them all again
     * The @p elements will be evenly divided between the threads
     * The benchmark is over once all elements have been consumed
     */
    static benchmark_result produce_and_consume(int threads, int elements) {
        benchmark_result results;
        if (threads <= 0) return results;

        LockFreeBag bag(threads);
        std::atomic_int consumed = 0;
        int e_per_p = elements/threads; // amount of elements to be produced by each thread
        elements = e_per_p*threads; // incase previous total amount was not evenly divisable over the threads

        #ifdef DEBUG
            std::cout << "Produce and consume with " << threads << " threads and " << elements << " elements" << std::endl;
            std::cout << "Each thread will produce " << e_per_p << " elements" << std::endl << std::endl;
        #endif
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            // first let every thread produce
            for (int e = 0; e < elements; e++) {
                bag.Add(id, e);
            }
            // then consume
            while (consumed < threads*elements) {
                data item = bag.TryRemoveAny(id);
                if (item != empty_data_val) {
                    consumed++;
                }
            }
        }
        results.time = omp_get_wtime() - t;

        for (int i = 0; i < threads; i++) {
            add_results(&results, bag.GetCounters(i));
            #ifdef DEBUG
                // std::cout << "Results from Thread: " << i << std::endl;
                // print_counters(bag.GetCounters(i));
            #endif
        }
        return results;
    }
};
