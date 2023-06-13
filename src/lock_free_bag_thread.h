#include <omp.h>
#include "LinkedList.h"
#include "benchmark_data.h"

class LockFreeBagThread {
private:
    LockFreeLinkedList** block_list; // pointer to array of the linked list so this thread can access other blocks for stealing
    int block_list_size;
    LockFreeNode* thread_block; // threadBlock from the algorithm from the paper. Points to current block containing the array the thread is working on
    LockFreeNode* steal_block = nullptr;  // stealBlock from the algorithm. Points to the current block containing the array this thread is stealing from
    LockFreeNode *stealPre = nullptr;
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
        if (thread_head >= LockFreeNode::block_size || block_list[id]->head == nullptr || thread_block->Mark1) {
            //std::cout << id <<" add new node with element " << thread_head << std::endl; 
            thread_block = block_list[id]->insert_node(item, 0);
            thread_head = 1;
        }else{
            //std::cout << id <<" add new element " << thread_head << std::endl;
            thread_block->data_array[thread_head].store(item, WEAK_ORDER);
            thread_head++;
            
            
        }
        counters.items_added++;
    }
	
    data Steal(){
        //std::cout << id <<" try to steal from " << steal_from_id << std::endl; 
        // if steal_block == nullptr this is the first time trying to steal so we need to set everything up and try to find a linkedlist
        int linked_lists_attempted = 0;
        counters.attempted_steals++;
        while (true) {
            if (steal_block == nullptr) {
                stealPre = nullptr; // reset the stealPre if movig to other list
                steal_from_id = (steal_from_id + 1) % block_list_size;
                if (steal_from_id == id) {
                    steal_from_id = (steal_from_id + 1) % block_list_size;
                }
                steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
                linked_lists_attempted++;
                steal_head = 0;
                if (linked_lists_attempted == block_list_size) return empty_data_val; 

                // test if the found block is marked
            }else if (steal_block->Mark1)
            {
                // try to remove the marked node, by doing so, try to remove every marked node in that linked list
                // based on the returned value we know if ther is still a node in the list
                if(stealPre != nullptr){
                    counters.atempts_to_delete++;
                    steal_block = stealPre->deleteNext(steal_block);
                    steal_head = 0;
                    stealPre = nullptr;
                }else{
                    steal_block = nullptr;
                }
                
                /*
                if(block_list[steal_from_id]->deleteNode()){
                    steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
                    steal_head = 0;
                }else{
                    steal_block = nullptr;
                }
                */
                
                // if we iterated throw one node, we test if it is the head of the list
                // if so we continue to the next, otherwise we will mark it
            }else if (steal_head >= LockFreeNode::block_size) {
                if (steal_block == block_list[steal_from_id]->head.load(WEAK_ORDER)){
                    stealPre = steal_block;
                    steal_block = steal_block->next;
                    steal_head = 0;
                
                }else{
                    //if we mark a block, we musst be sure that ther is no further element in the block
                    steal_block->setMark1();
                    for (int i = 0; i < LockFreeNode::block_size; i++)
                    {
                        data item = steal_block->getDataAt(i);
                        if (item != empty_data_val) {
                            Add(item);
                            counters.items_recoverd++;
                            //std::cout << id << " found valid data item in list to delete form " << steal_from_id << std::endl;
                        }
                    }
                    
                    steal_head = 0;
                }
                   
               
            
            }else{
                
                 data item = steal_block->getDataAt(steal_head);
                if (item != empty_data_val) { // the CAS already happens in getDataAt so no need to that here, we only need to check if it was successful by comparing with empty_data_val
                    counters.successful_steals++;
                    //std::cout << "Steal from " << id << " of item from " << steal_from_id << " successful" << std::endl;
                    return item;
                } else {
                    steal_head++;
                }
            }
        }
    }




    data TryRemoveAny() {
        //std::cout << id <<" try to remove " << thread_head << std::endl; 
        counters.attempted_removes++;
		while(true){
           
            
            if (thread_block == nullptr || thread_head < 0 || thread_block->Mark1){
                return Steal();
            }

            thread_head--;
            if (thread_head < 0){
                thread_block->setMark1();
            }

            if (thread_block->Mark1)
            {
                counters.atempts_to_delete++;
                if (block_list[id]->deleteHeadNode()){
                    //the list is not empty
                    thread_head = LockFreeNode::block_size -1;
                    thread_block = block_list[id]->head.load(std::memory_order_relaxed);
                }else{
                    //the local bag is empty, try to steal
                    return Steal();
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
