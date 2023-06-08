#ifndef _LINKEDLIST_
#define _LINKEDLIST_

/**
 * LinkedList for the Lock Free Bag Algorithm
 * Each thread uses its own linked list to store data in which is done in array blocks 
 */

#include "data_object.h"
#include <atomic>

static const int General_block_size = 32;

class LockBasedNode
{
private:
    bool Mark1 = false;
public:
    static const int block_size = General_block_size;
    data data_array[block_size];
    struct LockBasedNode *next = nullptr;
    omp_lock_t lock;
    

    LockBasedNode() {
        omp_init_lock(&lock);
        for (int i = 0; i < block_size; i++)
        {
            data_array[i] = empty_data_val;
        }
    }
    void setMark1(){
        omp_set_lock(&lock);
        Mark1 = true;
        omp_unset_lock(&lock);
    }

    bool getMark1(){
        return Mark1;
        
    }

    data get(int pos){
        data temp;
        omp_set_lock(&lock);
        temp = data_array[pos];
        data_array[pos] = empty_data_val;
        omp_unset_lock(&lock);
        return temp;
    }

    void set(int pos, data item) {
        omp_set_lock(&lock);
        data_array[pos] = item;
        omp_unset_lock(&lock);
    }
};

class LockBasedLinkedList {
private:
    omp_lock_t lock;
public:
    LockBasedLinkedList(){
        head = nullptr;
        omp_init_lock(&lock);
    }

    LockBasedNode* head = nullptr;
    

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
     LockBasedNode * insert_node() {
        LockBasedNode *new_node = new LockBasedNode();
        omp_set_lock(&lock);
        new_node->next = head;
        head = new_node;
        omp_unset_lock(&lock);
        return head;
    }

    

    LockBasedNode * insert_node(data toInsert, int pos) {
        LockBasedNode *new_node = new LockBasedNode();
        omp_set_lock(&lock);
        new_node->next = head;
        head = new_node;
        new_node->set(pos, toInsert);
        omp_unset_lock(&lock);
        
        return head;
    }

};


class LockFreeNode
{   
public:
    static const int block_size = General_block_size;
    atomic_data data_array[block_size];
    struct LockFreeNode *next = nullptr;
    std::atomic_bool Mark1 = false;
    LockFreeNode(){
        for (int i = 0; i < block_size; i++)
        {
            data_array[i] = empty_data_val;
        }
    }

    void setMark1(){
        std::atomic_exchange(&Mark1, true);
    }
    


    data getDataAt(int pos){
        data temp = data_array[pos];
        if (std::atomic_compare_exchange_weak(&data_array[pos], &temp, empty_data_val)){
            return temp;
        }else{
            return empty_data_val;
        }
    }
};



class LockFreeLinkedList {
public:
    LockFreeLinkedList(){
        head = nullptr;
    }

    std::atomic<LockFreeNode*> head = nullptr;

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
     LockFreeNode * insert_node() {
        LockFreeNode *new_node = new LockFreeNode();
        new_node->next = head;
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node)){
            new_node->next = head;
        }
        
        return head;
    }

    

    LockFreeNode * insert_node(data toInsert, int pos) {
        LockFreeNode *new_node = new LockFreeNode();
        new_node->data_array[pos].store(toInsert, WEAK_ORDER);

        new_node->next = head;
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node)){
            new_node->next = head;
        }
        
        return head;
    }

};

#endif