/**
 * LinkedList for the Lock Free Bag Algorithm
 * Each thread uses its own linked list to store data in which is done in array blocks 
 */

#include <omp.h>
#include "data_object.h"



class LinkedList_withLocks {

private: 
    omp_lock_t lock;
public:
    
    struct Node {
        static const int block_size = 10;
        //int array_block[block_size];
        data_object data_array[block_size];
        struct Node *next = nullptr;
    };

    LinkedList_withLocks(){
        head = nullptr;
        omp_init_lock(&lock);
    }

    Node* head = nullptr;

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
    Node* insert_node() {
        Node* new_node = new Node();
        //omp_set_lock(&lock);
        if (head == nullptr) {
            head = new_node;
            //omp_unset_lock(&lock);
            return head;
        }else{
            new_node->next = head;
            head = new_node;
            //omp_unset_lock(&lock);
            return new_node;
        }
    }


    void unlock_list(){
        omp_unset_lock(&this->lock);
    }
    void lock_list(){
        omp_set_lock(&this->lock);
    }

    void printLinkedList(){
        Node * currend = head;
        int listCounter = 0;
        while (currend != nullptr)
        {
            printf("List %d:",listCounter++);
            for (size_t i = 0; i < currend->block_size; i++)
            {
                if (currend->data_array[i].isValid())
                {
                    printf(" %2d ",currend->data_array[i].getData());
                }else{
                    printf(" -- ");
                }
            }
            printf("\n");
            currend = currend->next;
            
        }
        
    }

    /**
     * Removes node from end of list and returns the pointer to the new last node
     */
    Node* remove_node() {
        //omp_set_lock(&lock);
        if (head == nullptr) {
            omp_unset_lock(&lock);
            return nullptr;
        }
        
        head = head->next;
        //omp_unset_lock(&lock);
        return head;
    }

};



