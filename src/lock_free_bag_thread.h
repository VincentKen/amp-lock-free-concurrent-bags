#include <omp.h>

/**
 * LinkedList for the Lock Free Bag Algorithm
 * Each thread uses its own linked list to store data in which is done in array blocks 
 */
class LinkedList {
public:
    struct Node {
        static const int block_size = 10;
        int array_block[block_size];
        struct Node *next = nullptr;
    };

    Node* head = nullptr;

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
    Node* insert_node() {
        Node* new_node = new Node();

        if (head == nullptr) {
            head = new_node;
            return head;
        }

        Node* temp = head;
        while (temp->next != nullptr) {
            temp = temp->next;
        }

        temp->next = new_node;
        return new_node;
    }

    /**
     * Removes node from end of list and returns the pointer to the new last node
     */
    Node* remove_node() {
        if (head == nullptr) {
            return nullptr;
        }

        Node* temp1;
        Node* temp2 = nullptr;
        temp1 = head;
        while (temp1->next != nullptr) {
            temp2 = temp1;
            temp1 = temp1->next;
        }

        if (temp1 == head) {
            delete head;
            head = nullptr;
            return nullptr;
        }

        delete temp1;
        temp2->next = nullptr;
        return temp2;
    }
};


class LockFreeBagThread {
private:
    LinkedList** block_list; // pointer to array of the linked list so this thread can access other blocks for stealing
    int block_list_size;
    LinkedList::Node* thread_block; // threadBlock from the algorithm from the paper. Points to current block containing the array the thread is working on
    LinkedList::Node* steal_block;  // stealBlock from the algorithm. Points to the current block containing the array this thread is stealing from
    int thread_head; // threadHead from the algorithm. Index of the element in the array of the block where this thread will place data next
    int steal_head; // stealHead from the algorithm. Index of the element in the array of the stealBlock which this thread will steal next
    int id; // thread_id
public:

    LockFreeBagThread(int id, LinkedList** block_list, int block_list_size) {
        this->id = id;
        this->block_list = block_list;
        this->block_list_size = block_list_size;
        thread_block = block_list[id]->insert_node();
        thread_head = 0;
        // Barrier to force OMP to start all threads at the same time
        #pragma omp barrier
    }

    void Add(int item) {
        if (thread_head == LinkedList::Node::block_size) {
            thread_block = block_list[id]->insert_node();
            thread_head = 0;
        }
        thread_block->array_block[thread_head] = item;
        thread_head++;
    }

    void TryRemoveAny() {}
};
