#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

#include "lock_base_bag.h"



class lock_based_programs
{
private:
    /* data */
public:

    struct counters random_bench1(LockBasedBag bag, int times, int seed) {
        struct counters local_counter;
        int tid = omp_get_thread_num();
        printf("Thread %d started.\n", tid);

        // Barrier to force OMP to start all threads at the same time
        #pragma omp barrier

        struct random_data rand_state;
        int choice;
        char statebuf[32];
        bzero(&rand_state, sizeof(struct random_data));
        bzero(&statebuf,   sizeof(statebuf));
        initstate_r(seed, statebuf, 32, &rand_state);
        random_r(&rand_state, &choice);
        printf("Thread: %d, choise %d\n",tid, choice);
        
        for (int i = 0; i < times; i++)
        {
            random_r(&rand_state, &choice);
            if (choice % 2 == 0)
            {
                bag.add(tid,choice);
                local_counter.items_added++;
            }else{
                data_object newData = bag.TryRemoveAny(tid);
                        if (newData.isValid()){
                            local_counter.successful_removes++;
                        }else{
                            local_counter.failed_removes++;
                        }
            }
            
        }
        
        
        return local_counter;
    }

            counters testStealing(LockBasedBag bag, int threadID, int numberOfElements){
                struct counters local_counter;
                local_counter.successful_removes = 0;
                local_counter.failed_removes = 0;
                #pragma omp barrier

                if (threadID % 2 == 0)
                {
                    for (int i = 0; i < numberOfElements / 2; i++)
                    {
                        //printf("thread %d, add: %d\n",threadID,i);
                        bag.add(threadID, i);
                        local_counter.items_added++;
                    }
                }else{
                    for (int i = 0; i < numberOfElements / 2; i++)
                    {
                        //printf("thread %d, i= %d\n",threadID,i);
                        data_object newData = bag.TryRemoveAny(threadID);
                        if (newData.isValid()){
                            //printf("thread %d, removed: %d\n",threadID,newData.getData());
                            local_counter.successful_removes++;
                        }else{
                            //printf("thread %d, remove unsuccessfull\n",threadID);
                            local_counter.failed_removes ++;
                            //i--;
                        }
                    }
                }
                return local_counter;
                
            }



            counters addAndRemoveData(LockBasedBag bag, int threadID, int numberOfElements){
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
                LockBasedBag bag(numOfThreads);

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
                    result.reduced_counters.items_added += local_counters[thread].items_added;
                }
                
                result.time = toc -tic;
                return result;
            }


};