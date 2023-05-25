#include <omp.h>
#include "LinkedList.h"


//data_object NULL_Data_Object = data_object();

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

    int numOfStealOPs = 0;
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

    int getNumOfSteals(){
        return numOfStealOPs;
    }
///---------------------------------

    void Add(int item) {
        data_object* toAdd = new data_object(item);
        if (thread_head == Node::block_size || block_list[id]->head == nullptr || thread_block->Mark1) {
            thread_block = block_list[id]->insert_node(toAdd,0);
            thread_head = 1;
        }else{
            thread_block->data_array[thread_head] = toAdd;
            thread_head++;
        }
    }
	
    data_object* Steal(){
        numOfStealOPs++;
        int numOfTestedLists = 0;
        while (true)
        {
            if(steal_block == nullptr){
                steal_from_id = (steal_from_id + 1) % block_list_size;
                steal_block = block_list[steal_from_id]->head;
                steal_head = 0;

            }else if(steal_block->Mark1){
                if(block_list[steal_from_id]->remove_node()){
                    steal_block = block_list[steal_from_id]->head;
                }else{
                    steal_block = nullptr;
                }
                steal_head = 0;
            }else{
                if (steal_head >= Node::block_size)
                {
                    steal_block->setMark1();
                }else{
                    data_object *item = steal_block->getObjectAt(steal_head);
                    if (item != NULL_Data_Object){
                        return item;
                    }else{
                        steal_head++;
                    }
                }
            }
            if (numOfTestedLists >= 2* block_list_size)  return NULL_Data_Object;

            numOfTestedLists++;                 
        }
    }




    data_object* TryRemoveAny() {
		while(true){
            thread_head--;
            if (thread_head < 0){
                thread_block->setMark1();
                //printf("set Mark done %d\n",id);
                //fflush(stdout);
            }
            if (thread_block->Mark1)
            {
                // remove unsuccessfull & last node 
                if (!block_list[id]->remove_node() && block_list[id]->head.load(std::memory_order_relaxed)->next == nullptr){
                        return Steal();
                }else{
                    thread_head = Node::block_size -1;
                    thread_block = block_list[id]->head.load(std::memory_order_relaxed);
                }
                     
            }
            data_object *item = thread_block->getObjectAt(thread_head);
			if (item->isValid()){
				return item;
            }
		}
	}
};
