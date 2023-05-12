#include <omp.h>
#include "lock_free_bag_thread.h"

class LockFreeBag {
public:
    LinkedList* block_array;
    int threads;
public:
    LockFreeBag(int threads = 1) {
        this->threads = threads;
        block_array = new LinkedList[threads];

        omp_set_num_threads(threads);
        #pragma omp parallel for
        for (int i = 0; i < threads; i++) {
           LockFreeBagThread(i, &block_array, threads);
        }
    }

};