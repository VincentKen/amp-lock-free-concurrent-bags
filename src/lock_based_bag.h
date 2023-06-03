//
#ifndef _BAG_
#define _BAG_

#include <vector>
#include <omp.h>
#include "lock_based_bag_thread.h"

class LockBasedBag {
private:
    LockBasedLinkedList** block_array;
    int threads;
    LockBasedBagThread *bags_array;
public:
    LockBasedBag(){};

    LockBasedBag(int threads) 
    {
        this->threads = threads;
        this->block_array = new LockBasedLinkedList *[threads];
        bags_array = new LockBasedBagThread[threads];
        for (int i = 0; i < threads; i++) {
           bags_array[i] = LockBasedBagThread(i, block_array, threads);
        }
    }

    void Add(int thread_id, int item){
        bags_array[thread_id].Add(item);
    }

    data TryRemoveAny(int thread_id){
        return bags_array[thread_id].TryRemoveAny();
    }

};
#endif