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
    benchmark_result split_50_50_single(int threads) {
        benchmark_result results;
        if (threads % 2 != 0) threads--; // make sure there is an even amount of threads
        if (threads <= 0) return results;

        LockFreeBag bag(threads);
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel for
        for (int i = 0; i < threads; i++) {
            if (i % 2 == 0) {
                bag.Add(i, i);
            } else {
                bag.TryRemoveAny(i);
            }
        }
        results.time = omp_get_wtime() - t;

        for (int i = 0; i < threads; i++) {
            std::cout << "Thread " << i << " counters:" << std::endl;
            bag.GetCounters(i).print();
            results.add_results(bag.GetCounters(i));
        }

        return results;
    }

    /**
     * One of the threads will be designated as producer and all the others will be consumers
     * The producer will produce @p elements once all these elements have been consumed the benchmark is over
     * @param threads   The number of threads (must be >= 2)
     * @param elements  The number of elements to be produced by the producer 
     */
    benchmark_result single_producer(int threads, int elements) {
        benchmark_result results;
        if (threads < 2) return results;
        LockFreeBag bag(threads);

        std::atomic<bool> done = false;
        int completed[threads] = {0}; // keeps track of how many items each thread has consumed
        std::vector<int> consumed[threads]; // keeps track of WHICH items each thread has consumed
        omp_set_num_threads(threads + 1); // we add one thread which checks if all elements have been consumed
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id == 0) { // code ran by producer
                for (int e = 0; e < elements; e++) {
                    bag.Add(id, e);
                }
            } else if (id == threads) { // last thread checks if everything has been consumed
                while(!done) {
                    int total = 0;
                    for (int i = 0; i < threads; i++) {
                        total+= completed[i];
                    }
                    if (total >= elements) {
                        done = true;
                    }
                }
            } else { // code ran by the consumer threads
                while (!done) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        completed[id]++;
                        consumed[id].emplace_back(item);
                    }
                }
            }
        }
        results.time = omp_get_wtime() - t;
        for (int i = 0; i < threads; i++) {
            std::cout << "Thread " << i << " has consumed " << completed[i] << " items" << std::endl;
            bag.GetCounters(i).print();
            results.add_results(bag.GetCounters(i));
        }
        std::cout << "Benchmark took " << results.time << std::endl;
        return results;
    }

    /**
     * One of the threads will be designated as consumer and all the others will be producers
     * The producers will produce @p elements meaning there will in total be @p elements * (@p threads - 1) elements.
     * Once all these elements have been consumed the benchmark is over
     * @param threads   The number of threads (must be >= 2)
     * @param elements  The number of elements to be produced by each producer 
     */
    benchmark_result single_consumer(int threads, int elements) {
        benchmark_result results;
        if (threads < 2) return results;
        LockFreeBag bag(threads);

        int consumed = 0;
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id == 0) { // code ran by consumer
                while (consumed != elements*(threads-1)) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        consumed++;
                    }
                }
                
            } else { // code ran by the producer threads
                for (int e = 0; e < elements; e++) {
                    bag.Add(id, e);
                }
            }
        }
        results.time = omp_get_wtime() - t;
        for (int i = 0; i < threads; i++) {
            bag.GetCounters(i).print();
            results.add_results(bag.GetCounters(i));
        }
        return results;
    }

    /**
     * 50% of threads will be produces and 50% will be consumers
     * Each producer only produces 1 item
     */
    benchmark_result split_50_50(int threads, int elements) {
        benchmark_result results;
        if (threads % 2 != 0) threads--; // make sure there is an even amount of threads
        if (threads <= 0) return results;

        LockFreeBag bag(threads);
        std::atomic_int consumed = 0;
        omp_set_num_threads(threads);
        float t = omp_get_wtime();
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            #pragma omp barrier
            if (id % 2 == 0) { // produce
                for (int e = 0; e < elements; e++) {
                    bag.Add(id, e);
                }
            } else { // consume
                while (consumed != (threads/2)*elements) {
                    data item = bag.TryRemoveAny(id);
                    if (item != empty_data_val) {
                        consumed++;
                    }
                }
            }
        }
        results.time = omp_get_wtime() - t;

        for (int i = 0; i < threads; i++) {
            std::cout << "Thread " << i << " counters:" << std::endl;
            bag.GetCounters(i).print();
            results.add_results(bag.GetCounters(i));
        }
        std::cout << "Consumed: " << consumed << std::endl;
        return results;
    }
};