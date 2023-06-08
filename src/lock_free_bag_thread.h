#include <omp.h>
#include "LinkedList.h"
#include "benchmark_data.h"

class LockFreeBagThread {
private:
    LockFreeLinkedList** block_list; // pointer to array of the linked list so this thread can access other blocks for stealing
    int block_list_size;
    LockFreeNode* thread_block; // threadBlock from the algorithm from the paper. Points to current block containing the array the thread is working on
    LockFreeNode* steal_block = nullptr;  // stealBlock from the algorithm. Points to the current block containing the array this thread is stealing from
    int thread_head; // threadHead from the algorithm. Index of the element in the array of the block where this thread will place data next
    int steal_head; // stealHead from the algorithm. Index of the element in the array of the stealBlock which this thread will steal next
    int id; // thread_id
    int steal_from_id;
    benchmark_counters counters;
public:

    LockFreeBagThread(){}


    LockFreeBagThread(int id, LockFreeLinkedList** block_list, int block_list_size) {
        this->id = id;
        this->block_list = block_list;
        this->block_list_size = block_list_size;
        block_list[id] = new LockFreeLinkedList();
        thread_block = block_list[id]->insert_node();
        thread_head = 0;
        steal_from_id = (id + 1) % block_list_size;
        steal_head = 0;
    }
    
    benchmark_counters GetCounters(){
        return counters;
    }

    void Add(data item) {
        //std::cout << id <<" add at " << thread_head << std::endl; 

        if (thread_head == LockFreeNode::block_size || block_list[id]->head == nullptr || thread_block->Mark1) {
            thread_block = block_list[id]->insert_node(item, 0);
            thread_head = 1;
        }else{
            thread_block->data_array[thread_head].store(item, WEAK_ORDER);
            thread_head++;
            if(thread_block->Mark1){
                thread_block = block_list[id]->insert_node(item, 0);
                thread_head = 1;
            }
            
        }
        counters.items_added++;
    }
	
    data Steal(){
        //std::cout << id <<" try to steal from " << steal_from_id << std::endl; 
        // if steal_block == nullptr this is the first time trying to steal so we need to set everything up and try to find a linkedlist
        int linked_lists_attempted = 0;
        while (true) {
            if (steal_block == nullptr) {
                steal_from_id = (steal_from_id + 1) % block_list_size;
                if (steal_from_id == id) {
                    steal_from_id = (steal_from_id + 1) % block_list_size;
                }
                steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
                linked_lists_attempted++;
                steal_head = 0;
                if (linked_lists_attempted == block_list_size) return empty_data_val; 
            }else if (steal_block->Mark1)
            {
                //std::cout << "Delete stealing " << id << " from " << steal_from_id << std::endl;
                if(block_list[steal_from_id]->deleteNode()){
                    std::cout << "Delete stealing " << id << " from " << steal_from_id << " succsessfull" << std::endl;
                    steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
                    steal_head = 0;
                }else{
                    //std::cout << "Delete stealing " << id << " from " << steal_from_id << " failed" << std::endl;
                    std::cout << id << " Going to next list. Attempts so far: " << linked_lists_attempted << "/" << block_list_size << std::endl;
                    steal_block = nullptr;
                }
                
                
            }else if (steal_head >= LockFreeNode::block_size) {
                steal_block->setMark1();
            
            }else{
                 data item = steal_block->getDataAt(steal_head);
                if (item != empty_data_val) { // the CAS already happens in getDataAt so no need to that here, we only need to check if it was successful by comparing with empty_data_val
                    counters.successful_steals++;
                    std::cout << "Steal from " << id << " of item from " << steal_from_id << " successful" << std::endl;
                    return item;
                } else {
                    steal_head++;
                }
            }
        }
    }




    data TryRemoveAny() {
        //std::cout << id <<" try to remove " << thread_head << std::endl; 
		while(true){
            counters.attempted_removes++;

            if (thread_head < 0 && thread_block->Mark1){
                return Steal();
            }

            thread_head--;
            if (thread_head < 0){
                thread_block->setMark1();
            }

            if (thread_block->Mark1)
            {
                //std::cout << "Delete " << id << " from it selve" << std::endl;
                // remove unsuccessfull & last node 
               if (!block_list[id]->deleteNode() && block_list[id]->head.load(std::memory_order_relaxed)->next == nullptr){
                    std::cout << " Thread " << id << " failed to delete from itself" << std::endl;
                    return Steal();
                }else{
                    thread_head = LockFreeNode::block_size -1;
                    thread_block = block_list[id]->head.load(std::memory_order_relaxed);
                }
                     
            }

            data item = thread_block->getDataAt(thread_head);
            if (item != empty_data_val) { // the CAS already happened in getDataAt, only check if it was successful by comparing with empty_data_val
                counters.successful_removes++;
                return item;
            } 

		}
	}
};
