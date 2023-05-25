#include <omp.h>
#include "LinkedList_Locks.h"


//data_object NULL_Data_Object = data_object();

class LockBasedBagThread {
private:
    LinkedList_withLocks** block_list; // pointer to array of the linked list so this thread can access other blocks for stealing
    int block_list_size;
    LinkedList_withLocks::Node* thread_block; // threadBlock from the algorithm from the paper. Points to current block containing the array the thread is working on
    LinkedList_withLocks::Node* steal_block;  // stealBlock from the algorithm. Points to the current block containing the array this thread is stealing from
    int thread_head; // threadHead from the algorithm. Index of the element in the array of the block where this thread will place data next
    int steal_head; // stealHead from the algorithm. Index of the element in the array of the stealBlock which this thread will steal next
    int id; // thread_id
    int steal_from_id;
public:

    LockBasedBagThread(){}


    LockBasedBagThread(int id, LinkedList_withLocks** block_list, int block_list_size) {
        this->id = id;
        this->block_list = block_list;
        this->block_list_size = block_list_size;
        block_list[id] = new LinkedList_withLocks();
        thread_block = block_list[id]->insert_node();
        thread_head = 0;
        steal_from_id = 0;
        // Barrier to force OMP to start all threads at the same time
        //printf("thread bag %d initialized. bagptr: %d\n",id,&block_list[id]);
        //#pragma omp barrier
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
///---------------------------------

    void Add(int item) {
        block_list[id]->lock_list();
        if (thread_head == LinkedList_withLocks::Node::block_size || block_list[id]->head == nullptr) {
            thread_block = block_list[id]->insert_node();
            thread_head = 0;
        }
        data_object toAdd = data_object(item);
        thread_block->data_array[thread_head] = toAdd;
        block_list[id]->unlock_list();
        thread_head++;
    }
	
    data_object Steal(){
        while (true)
        {
            //printf("stealing from: %d, header: %d\n",steal_from_id, steal_head);

            int stepsInLoop = 0;
            while (steal_block == nullptr)
            {
                steal_head = 0;
                steal_from_id = (steal_from_id + 1) % block_list_size;
                steal_block = block_list[steal_from_id]->head;
                stepsInLoop++;
                if (stepsInLoop > block_list_size)
                {
                    return NULL_Data_Object;
                }
                
            } 
            block_list[steal_from_id]->lock_list();
            if (steal_head >= LinkedList_withLocks::Node::block_size)
            {
                steal_block = block_list[steal_from_id]->remove_node();
                steal_head = 0;
            }else{
                data_object item;
                //#pragma omp atomic capture
                item = steal_block->data_array[steal_head];
                steal_block->data_array[steal_head] = NULL_Data_Object;
                
                if (item.isValid()){
                    block_list[steal_from_id]->unlock_list();		
                    return item;
                }else{
                    steal_head++;
                }
            }
            block_list[steal_from_id]->unlock_list();
            
        }
        
        
        
       
        
    }

    data_object TryRemoveAny() {
		while(true){
            // could check for an element outsid the node array
            if (thread_head >= LinkedList_withLocks::Node::block_size)
            {
                thread_head--;
            }

            if(thread_block == nullptr){ //steal
                //printf("steal Mode Thread %d\n",id);
                return Steal();
            }

			if (thread_head < 0){	//block is empty
                block_list[id]->lock_list();
				thread_block = block_list[id]->remove_node();
                block_list[id]->unlock_list();

				if(thread_block == nullptr){	//is the last block, local thread list is empty
					//steal
                    return Steal();
				}
                thread_head = LinkedList_withLocks::Node::block_size -1;

			}
			
			data_object item;
			
            block_list[id]->lock_list();
            item = thread_block->data_array[thread_head];
            thread_block->data_array[thread_head] = NULL_Data_Object;
		
			if (item.isValid()){		//check if a data item is at that position, if yes try swap
                block_list[id]->unlock_list();
				return item;
			}else{
				thread_head--;
			}
			block_list[id]->unlock_list();
		}
	}
};
