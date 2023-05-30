/**
 * LinkedList for the Lock Free Bag Algorithm
 * Each thread uses its own linked list to store data in which is done in array blocks 
 */

#include "data_object.h"
#include <atomic>

class Node
{   
public:
    static const int block_size = 10;
    atomic_data data_array[block_size];
    struct Node *next = nullptr;
    std::atomic_bool Mark1 = false;
    Node(){
        for (int i = 0; i < block_size; i++)
        {
            data_array[i] = empty_data_val;
        }
    }

    void setMark1(){
        std::atomic_exchange(&Mark1, true);
    }


    data getDataAt(int pos){
        data temp = data_array[pos].load(WEAK_ORDER);
        
        if (std::atomic_compare_exchange_weak(&data_array[pos], &temp, empty_data_val)){
            return temp;
        }else{
            return empty_data_val;
        }
    }
};



class LinkedList {
public:
    LinkedList(){
        head = nullptr;
    }

    std::atomic<Node*> head = nullptr;

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
     Node * insert_node() {
        Node *new_node = new Node();
        new_node->next = head;
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node)){
            new_node->next = head;
        }
        
        return head;
        
    }

    

    Node * insert_node(data toInsert, int pos) {
        Node *new_node = new Node();
        new_node->data_array[pos].store(toInsert, WEAK_ORDER);

        new_node->next = head;
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node)){
            new_node->next = head;
        }
        
        return head;
        
    }
    

    void printLinkedList(){
        Node * currend = head;
        int listCounter = 0;
        //printf("start printList\n");
        while (currend != nullptr)
        {
            printf("List %d, is Marked %d:",listCounter++, currend->Mark1.load(std::memory_order_relaxed)); // currend->Mark1.load(std::memory_order_relaxed));
            fflush(stdout);
            for (size_t i = 0; i < currend->block_size; i++)
            {
                data obj = currend->data_array[i].load(WEAK_ORDER);
                if (obj != empty_data_val)
                {
                    printf(" %d ", obj);
                    fflush(stdout);
                }else{
                    printf(" -- ");
                }
            }
            printf("\n");
            currend = currend->next;
            
        }
        
    }

    // bool remove_node() {
    //     Node * toRemove = head;
    //     Node *nextNode = head.load(std::memory_order_relaxed)->next;
    //     if (nextNode == nullptr) {
    //         return false;
    //     }
    //     return std::atomic_compare_exchange_weak(&head, &toRemove, nextNode);
    // }

};



