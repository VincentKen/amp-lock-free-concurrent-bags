/**
 * LinkedList for the Lock Free Bag Algorithm
 * Each thread uses its own linked list to store data in which is done in array blocks 
 */

class data_object
{
private:
    bool hasData = false;
    int data;
public:
    data_object(){
        hasData = false;
     }
    data_object(int data){
        hasData = true;
        this->data = data;
    }

    bool isValid(){
        return hasData;
    }
    int getData(){
        return data;
    } 

};




class LinkedList {
public:
    struct Node {
        static const int block_size = 10;
        //int array_block[block_size];
        data_object data_array[block_size];
        struct Node *next = nullptr;
    };

    LinkedList(){
        head = nullptr;
    }

    Node* head = nullptr;

    /**
     * Inserts new node to the end of the list and returns the pointer to that node 
     */
    Node* insert_node() {
        Node* new_node = new Node();

        if (head == nullptr) {
            head = new_node;
            return head;
        }else{
            new_node->next = head;
            head = new_node;
            return new_node;
        }
/*
        Node* temp = head;
        while (temp->next != nullptr) {
            temp = temp->next;
        }

        temp->next = new_node;
        return new_node;
*/
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
        if (head == nullptr) {
            return nullptr;
        }
        
        head = head->next;
        return head;
    }
/*
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
*/
};



