#include <omp.h>
#include "lock_free_bag_thread.h"

class LockFreeBag {
private:
    LockFreeLinkedList** block_array;
    int threads;
    LockFreeBagThread *bags_array;
public:
    LockFreeBag(){};

    LockFreeBag(int threads) 
    {
        this->threads = threads;
        this->block_array = new LockFreeLinkedList *[threads];
        bags_array = new LockFreeBagThread[threads];
        
        for (int i = 0; i < threads; i++) {
           bags_array[i] = LockFreeBagThread(i, block_array, threads);
        }
    }

    void Add(int thread_id, int item){
        bags_array[thread_id].Add(item);
    }

    data TryRemoveAny(int thread_id){
        return bags_array[thread_id].TryRemoveAny();
    }

    benchmark_counters GetCounters(int thread_id){
        return bags_array[thread_id].GetCounters();
    }



};
