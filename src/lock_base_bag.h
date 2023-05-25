#include <omp.h>
#include "lock_based_bag_thread.h"

class LockBasedBag {
//public:
    LinkedList_withLocks** block_array;
    int threads;
    LockBasedBagThread *bags_array;
public:
    LockBasedBag(){};

    LockBasedBag(int threads) 
    {
        this->threads = threads;
        this->block_array = new LinkedList_withLocks *[threads];
        bags_array = new LockBasedBagThread[threads];
        omp_set_num_threads(threads);
        
        #pragma omp parallel for
        for (int i = 0; i < threads; i++) {
		    //printf("init %d\n",i);
           bags_array[i] = LockBasedBagThread(i, block_array, threads);
        }
        
        #pragma omp barrier
    }

    void add(int thread_id, int item){
        bags_array[thread_id].Add(item);
    }

    data_object TryRemoveAny(int thread_id){
        return bags_array[thread_id].TryRemoveAny();
    }





//----------- debug methods---------------
    void insertNodeToLinkedList(int thread_id){
        bags_array[thread_id].insertNode();
    }

    void printLinkedListFromTread(int thread_id){
        bags_array[thread_id].printLinkedList();
    }
///---------------------------------




};
