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
    data_object *data_array[block_size];
    struct Node *next = nullptr;
    std::atomic_bool Mark1 = false;
    Node(){
        for (int i = 0; i < block_size; i++)
        {
            data_array[i] = NULL_Data_Object;
        }
    }

    void setMark1(){
        std::atomic_exchange(&Mark1, true);
    }


    data_object* getObjectAt(int pos){
        data_object *temp = new data_object(data_array[pos]->getData());
    
        if (std::atomic_exchange(&data_array[pos]->hasData, false)){
            return temp;
        }else{
            return NULL_Data_Object;
        }
    }
};



class LinkedList {
public:
 /*
    struct Node {
        static const int block_size = 10;
        //int array_block[block_size];
        data_object *data_array[block_size];
        
        
        struct Node *next = nullptr;
        std::atomic_bool Mark1 = false;
        bool Mark2 = false;

        void setMark1(){
            std::atomic_exchange(&Mark1, true);
        }


        data_object* getObjectAt(int pos){
            data_object *temp = new data_object(data_array[pos]->getData());
        
            if (std::atomic_exchange(&data_array[pos]->hasData, true)){
                return temp;
            }else{
                return NULL_Data_Object;
            }
        }
    };
*/

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

    

    Node * insert_node(data_object *toInsert, int pos) {
        Node *new_node = new Node();
        new_node->data_array[pos] = toInsert;

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
                
                if (currend->data_array[i]->isValid())
                {
                    printf(" %d ",currend->data_array[i]->getData());
                    fflush(stdout);
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
     
    Node* remove_node() {
        struct Node toRemove = head;
        struct Node nextNode = this->head->next();
        if (nextNode == nullptr) {
            return nullptr;
        }
        while (!std::atomic_compare_exchange_weak(head, new_node->next, new_node));
        return head;
    }
    */

    bool remove_node() {
        Node * toRemove = head;
        Node *nextNode = head.load(std::memory_order_relaxed)->next;
        if (nextNode == nullptr) {
            return false;
        }
        return std::atomic_compare_exchange_weak(&head, &toRemove, nextNode);
    }

};



