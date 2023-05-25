#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>
#include <iostream>
#include <fstream>

#include "lock_free_bag.h"



class lock_free_programs
{
private:
    /* data */
public:

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
            data_object *newData = bag.TryRemoveAny(threadID);
            if (newData->isValid()){
                
                //printf("thread %d, removed: %d\n",threadID,newData->getData());
                local_counter.successful_removes++;
            }else{
                //printf("thread %d, remove unsuccessfull\n",threadID);
                local_counter.failed_removes ++;
            }            
        }
    }
    local_counter.items_stolen = bag.numberOfStealOps(threadID);
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
        data_object *newData = bag.TryRemoveAny(threadID);
        if (newData->isValid()){
            local_counter.successful_removes++;
        }else{
            local_counter.failed_removes ++;
        }
    }
    return local_counter;
}

counters smalBanch(LockFreeBag bag, int threadID, int numberOfElements){
    struct counters local_counter;
    local_counter.successful_removes = 0;
    local_counter.failed_removes = 0;
    std::string fileName = "thread" + std::to_string(threadID) + ".txt";
    std::ofstream TreadFile(fileName);

    for (int i = 0; i < numberOfElements; i++)
    {
        int data = i*100 | threadID;
        bag.add(threadID, data);
        //printf("thread %d, added: %d\n", threadID, data);
        local_counter.items_added++;
    }
    printf("thread %d, done adding\n",threadID);
    data_object *newData = bag.TryRemoveAny(threadID);
    while (newData != NULL_Data_Object)
    {   
        //printf("thread %d, removed %d\n",threadID, newData->getData());
        local_counter.successful_removes++;
        TreadFile << std::to_string(newData->getData()) << std::endl;
        newData = bag.TryRemoveAny(threadID);
    }
    printf("thread %d, done removing\n",threadID);
    fflush(stdout);
    local_counter.items_stolen = bag.numberOfStealOps(threadID);
    return local_counter;
}

counters loop_Add_remove(LockFreeBag bag, int threadID, int numberOfElements){
    struct counters local_counter;
    local_counter.successful_removes = 0;
    local_counter.failed_removes = 0;
    for (int i = 0; i < numberOfElements; i++)
    {
        data_object *newData = bag.TryRemoveAny(threadID);
        if (newData->isValid()){            
            local_counter.successful_removes++;
            bag.add(threadID, newData->getData());
            local_counter.items_added++;

        }else{
            local_counter.failed_removes ++;
        }

        
    }
    local_counter.items_stolen = bag.numberOfStealOps(threadID);
    return local_counter;
}

struct bench_result loop_add_and_remove(int numOfThreads, int numElements) {
    struct bench_result result;
    struct counters local_counters[numOfThreads];
    LockFreeBag bag(numOfThreads);

    //load some elements in to the bag, so it is not empty
    for (int i = 0; i < numOfThreads +1 ; i++)
    {
        bag.add(0,i);
    }
    //bag.printLinkedListFromTread(0);
    omp_set_num_threads(numOfThreads);
    double tic = omp_get_wtime();

    #pragma omp parallel for
    for (int thread = 0; thread < numOfThreads; thread++)
    {
        //local_counters[thread] = addAndRemoveData(bag,thread,numElements/numOfThreads); 
        local_counters[thread] = addAndRemoveData(bag,thread,numElements);       
    }
    #pragma omp barrier

    double toc = omp_get_wtime();
    result.reduced_counters.successful_removes = 0;
    result.reduced_counters.failed_removes = 0;
    
    for (int thread = 0; thread < numOfThreads; thread++){
        result.reduced_counters.failed_removes += local_counters[thread].failed_removes;
        result.reduced_counters.successful_removes += local_counters[thread].successful_removes;
        result.reduced_counters.items_added += local_counters[thread].items_added;
        result.reduced_counters.items_stolen += local_counters[thread].items_stolen;
        //printf("counters from %d, sucsessfull: %d, faild %d\n", thread, local_counters[thread].successful_removes, local_counters[thread].failed_removes);
    }
    
    result.time = toc -tic;
    return result;

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
        //local_counters[thread] = testStealing(bag,thread,numElements);
        local_counters[thread] = smalBanch(bag,thread,numElements);       
    }
    #pragma omp barrier
    
    result.reduced_counters.successful_removes = 0;
    result.reduced_counters.failed_removes = 0;
    double toc = omp_get_wtime();
    for (int thread = 0; thread < numOfThreads; thread++){
        result.reduced_counters.failed_removes += local_counters[thread].failed_removes;
        result.reduced_counters.successful_removes += local_counters[thread].successful_removes;
        result.reduced_counters.items_added += local_counters[thread].items_added;
        result.reduced_counters.items_stolen += local_counters[thread].items_stolen;
        printf("counters from %d, sucsessfull: %d, faild %d\n", thread, local_counters[thread].successful_removes, local_counters[thread].failed_removes);
    }
    
    result.time = toc -tic;
    return result;
}

};