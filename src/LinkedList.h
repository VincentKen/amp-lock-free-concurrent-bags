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
    std::atomic<LockFreeNode*> next = nullptr;
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

//used for the compare and exchange operation, has problems when just given a nullpointer
LockFreeNode *null_node = nullptr;

class LockFreeLinkedList {
public:
    LockFreeLinkedList(){
        head = nullptr;
    }

    std::atomic<LockFreeNode*> head = new LockFreeNode;


    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
     LockFreeNode * insert_node() {
        LockFreeNode *new_node = new LockFreeNode();
        LockFreeNode *h = head.load(WEAK_ORDER);
        
        std::atomic_compare_exchange_weak(&new_node->next, &null_node, h);
        
        while (!std::atomic_compare_exchange_weak(&head, &h, new_node)){
            LockFreeNode *h_old = h;
            h = head.load(WEAK_ORDER);
            std::atomic_compare_exchange_weak(&new_node->next, &h_old, h);
        }        
        return new_node;
    }

    LockFreeNode * insert_node(data toInsert, int pos) {
        // init a new node and add the data element
        LockFreeNode *new_node = new LockFreeNode();
        new_node->data_array[pos].store(toInsert, WEAK_ORDER);
        
        // set the next pointer of the new node to the head
        LockFreeNode *h = head.load(std::memory_order_relaxed);
        std::atomic_compare_exchange_weak(&new_node->next, &null_node, h);

        // exchange the head variable with the new node
        while (!std::atomic_compare_exchange_weak(&head, &h, new_node)){
            LockFreeNode *h_old = h;
            h = head.load(std::memory_order_relaxed);
            std::atomic_compare_exchange_weak(&new_node->next, &h_old, h);
        }

        return new_node;
    }
    

    // returns true if the list still has a node avter deleting
    bool deleteNode(){
        LockFreeNode *headNode = head;
        while (headNode != nullptr && headNode->Mark1)
        {
            std::atomic_compare_exchange_weak(&head, &headNode, headNode->next);
            headNode = head;
        }

        
        while (headNode != nullptr)
        {
            LockFreeNode *next = headNode->next;
            if (next == nullptr){
                return true;

            }else if(next->Mark1){
                std::atomic_compare_exchange_weak(&headNode->next, &next, next->next);

            }else{
                headNode = next;
            }
        }
        return false;       
    }

    void printList(){
        LockFreeNode *iter = head.load(WEAK_ORDER);
        int cnt = 0; 
        std::cout << "start printing list"<< std::endl;
        while (iter != nullptr)
        {
            std::cout << "Node: " << cnt++ << " [  is marked: " << iter->Mark1 << " ]"<< std::endl;
            iter = iter->next;
        }
        
    }

};

#endif