#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

#include "lock_free_programs.h"

struct benchmark_result small_bench(int t, int len) {
    lock_free_programs lock_free;
    return lock_free.split_50_50(t, len);
}

// void printResults(int numOfThreads,int numElements,  benchmark_result result){
//     printf("NumOfThreads: %d, NumofElements %d, Time: %f, itemsAdded: %d, sucsessfullRemoves: %d, FaildRemoves: %d, stolenItems: %d\n", numOfThreads,numElements, result.time, result.reduced_counters.items_added, result.reduced_counters.successful_removes, result.reduced_counters.failed_removes, result.reduced_counters.items_stolen);
// }

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    
    int t = 8;
    int numElements = 10000;
    benchmark_result test;
    test = small_bench(t, numElements);
    // printResults(t,numElements,test);


    /*
    
    
	printf("start\n");
    bench_result test;
    int t = 8;
    int numElements = 1000000;

    //lock_based_programs lock_based;
    lock_free_programs lock_free;

    //printf("with locks\n");
    //test = lock_based.small_bench(t, numElements);
    //printResults(t,numElements,test);

    printf("lock free\n");
    //test = lock_free.small_bench(t, numElements);
    //printResults(t,numElements,test);
    test = lock_free.small_bench(t, numElements);
    printResults(t,numElements,test);

    for (int t = 1; t < 0; t++)
    {
        for (int numElements = 1000; numElements < 10000000; numElements *= 10)
        {
            printf("New test, numT %d, numElem: %d\n", t, numElements);
            test = lock_free.small_bench(t, numElements);
            printResults(t,numElements,test);
        }
        printf("\n");
        
    }
    

    

    /*
    small_bench(2);
    
    
    small_bench(4);
    small_bench(8);
    small_bench(16);
    */   
}
