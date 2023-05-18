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
    int failed_removes;
    int successful_removes;
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


counters testStealing(LockFreeBag bag, int threadID, int numberOfElements){
    struct counters local_counter;
    local_counter.successful_removes = 0;
    local_counter.failed_removes = 0;


    if (threadID % 2 == 0)
    {
        for (int i = 0; i < numberOfElements / 2; i++)
        {
            //printf("thread %d, add: %d\n",threadID,i);
            bag.add(threadID, i);
        }
    }else{
        for (int i = 0; i < numberOfElements / 2; i++)
        {
            data_object newData = bag.TryRemoveAny(threadID);
            if (newData.isValid()){
                //printf("thread %d, removed: %d\n",threadID,newData.getData());
                local_counter.successful_removes++;
            }else{
                //printf("thread %d, remove unsuccessfull\n",threadID);
                local_counter.failed_removes ++;
            }
        }
    }
    return local_counter;
    
}



counters addAndRemoveData(LockFreeBag bag, int threadID, int numberOfElements){
    struct counters local_counter;
    local_counter.successful_removes = 0;
    local_counter.failed_removes = 0;

    for (int i = 0; i < numberOfElements; i++)
    {
        bag.add(threadID, i);
    }
    for (int i = 0; i < numberOfElements; i++)
    {
        data_object newData = bag.TryRemoveAny(threadID);
        if (newData.isValid()){
            local_counter.successful_removes++;
        }else{
            local_counter.failed_removes ++;
        }
    }
    return local_counter;
}


struct bench_result small_bench(int numOfThreads, int numElements) {
    struct bench_result result;
    struct counters local_counters[numOfThreads];
    LockFreeBag bag(numOfThreads);

    printf("bag is initialised: numOfThreads: %d\n",numOfThreads);
    
    omp_set_num_threads(numOfThreads);
    double tic = omp_get_wtime();

    #pragma omp parallel for
    for (int thread = 0; thread < numOfThreads; thread++)
    {
        //local_counters[thread] = addAndRemoveData(bag,thread,numElements/numOfThreads); 
        local_counters[thread] = testStealing(bag,thread,numElements);       
    }
    #pragma omp barrier

    result.reduced_counters.successful_removes = 0;
    result.reduced_counters.failed_removes = 0;
    double toc = omp_get_wtime();
    for (int thread = 0; thread < numOfThreads; thread++){
        result.reduced_counters.failed_removes += local_counters[thread].failed_removes;
        result.reduced_counters.successful_removes += local_counters[thread].successful_removes;
        printf("counters from %d, sucsessfull: %d, faild %d\n", thread, local_counters[thread].successful_removes, local_counters[thread].failed_removes);
    }
    
    result.time = toc -tic;
    return result;
}

void printResults(int numOfThreads,int numElements,  bench_result result){
    printf("NumOfThreads: %d, Time: %f, sucsessfullRemoves: %d, FaildRemoves: %d\n",numOfThreads,result.time,result.reduced_counters.successful_removes,result.reduced_counters.failed_removes);
}

/* main is not relevant for benchmark.py but necessary when run alone for
 * testing.
 */
int main(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
	printf("start\n");
    bench_result test;
    int t = 4;
    int numElements = 100;

    test = small_bench(t, numElements);
    printResults(t,numElements,test);

    for (int t = 0; t < 0; t++)
    {
        for (int numElements = 1000; numElements < 10000000; numElements *= 10)
        {
            test = small_bench(t, numElements);
            printResults(t,numElements,test);
        }
        printf("\n");
        
    }
    

    

    /*
    small_bench(2);
    
    /*
     * small_bench(4);
    small_bench(8);
    small_bench(16);
    */   
}
