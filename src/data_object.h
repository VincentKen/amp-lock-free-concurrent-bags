#ifndef data_object_h
#define data_object_h

#include <atomic>





class data_object
{
private:
   
    int data;
    
public:
std::atomic_bool hasData = false;

    data_object(){
        hasData = false;
     }
    data_object(int data){
        hasData = true;
        this->data = data;
    }
    data_object(int data, bool hasData){
        this->hasData = hasData;
        this->data = data;
    }

    bool isValid(){
        return hasData;
    }
    int getData(){
        return data;
    } 
    

};

struct counters {
    int failed_removes  = 0;
    int successful_removes = 0;
    int items_added = 0;
    int items_stolen = 0;
};
struct bench_result {
    float time;
    struct counters reduced_counters;
};

data_object * NULL_Data_Object = new data_object();



#endif