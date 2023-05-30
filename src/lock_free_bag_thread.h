#include <omp.h>
#include "LinkedList.h"
#include "benchmark_data.h"

class LockFreeBagThread {
private:
    LinkedList** block_list; // pointer to array of the linked list so this thread can access other blocks for stealing
    int block_list_size;
    Node* thread_block; // threadBlock from the algorithm from the paper. Points to current block containing the array the thread is working on
    Node* steal_block = nullptr;  // stealBlock from the algorithm. Points to the current block containing the array this thread is stealing from
    int thread_head; // threadHead from the algorithm. Index of the element in the array of the block where this thread will place data next
    int steal_head; // stealHead from the algorithm. Index of the element in the array of the stealBlock which this thread will steal next
    int id; // thread_id
    int steal_from_id;
    benchmark_counters counters;
public:

    LockFreeBagThread(){}


    LockFreeBagThread(int id, LinkedList** block_list, int block_list_size) {
        this->id = id;
        this->block_list = block_list;
        this->block_list_size = block_list_size;
        block_list[id] = new LinkedList();
        thread_block = block_list[id]->insert_node();
        thread_head = 0;
        steal_from_id = 0;
        steal_head = 0;
    }

//----------- debug methods---------------
    void printLinkedList(){
        printf("printList from thread %d:\n",id);
        block_list[id]->printLinkedList();
        
    }
    void insertNode(){
        thread_block = block_list[id]->insert_node();
        thread_head = 0;
    }

    benchmark_counters GetCounters(){
        return counters;
    }
///---------------------------------

    void Add(data item) {
        if (thread_head == Node::block_size || block_list[id]->head == nullptr || thread_block->Mark1) {
            thread_block = block_list[id]->insert_node(item, 0);
            thread_head = 1;
        }else{
            thread_block->data_array[thread_head].store(item, WEAK_ORDER);
            thread_head++;
        }
        counters.items_added++;
    }
	
    data Steal(){
        // if steal_block == nullptr this is the first time trying to steal so we need to set everything up and try to find a linkedlist
        int rounds = 0;
        while (steal_block == nullptr) {
            steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
            steal_from_id = (steal_from_id + 1) % block_list_size;
            if (rounds >= 50*block_list_size) {
                return empty_data_val;
            }
            rounds++;
        }
        int linked_lists_attempted = 0;
        while (true) {
            counters.attempted_steals++;
            if (steal_head == Node::block_size) {
                if (linked_lists_attempted == 50*block_list_size) return empty_data_val; // bag must be empty. TODO implement method from paper
                steal_block = steal_block->next;
                steal_head = 0;
                if (steal_block == nullptr) { // reached end of of this linked list
                    steal_from_id = (steal_from_id + 1) % block_list_size;
                    steal_block = block_list[steal_from_id]->head.load(WEAK_ORDER);
                    linked_lists_attempted++;
                }
            }
            data item = steal_block->getDataAt(steal_head);
            if (item != empty_data_val) { // the CAS already happens in getDataAt so no need to that here, we only need to check if it was successful by comparing with empty_data_val
                counters.successful_steals++;
                return item;
            } else {
                steal_head++;
            }

        }
    }




    data TryRemoveAny() {
		while(true){
            counters.attempted_removes++;
            if (thread_head < 0){
                if (thread_block->next == nullptr) { // indicates it is last block in the linked list
                    return Steal();
                }
                thread_block = thread_block->next;
                thread_head = Node::block_size - 1;
            }

            data item = thread_block->getDataAt(thread_head);
            if (item != empty_data_val) { // the CAS already happened in getDataAt, only check if it was successful by comparing with empty_data_val
                counters.successful_removes++;
                return item;
            } else {
                thread_head--;
            }

		}
	}
};
