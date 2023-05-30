#ifndef data_object_h
#define data_object_h

#include <atomic>


#define WEAK_ORDER std::memory_order_relaxed // std::memory_order_relaxed is waaaay too long to type out every time


// class data_object
// {
// private:
//     int data;
// public:
//     std::atomic_bool hasData = false;

//     data_object(){
//         hasData = false;
//      }
//     data_object(int data){
//         hasData = true;
//         this->data = data;
//     }
//     data_object(int data, bool hasData){
//         this->hasData = hasData;
//         this->data = data;
//     }

    // data_object(data_object &obj) {
    //     data = obj.data;
    //     hasData = obj.hasData.load(WEAK_ORDER);
    // }

    // data_object& operator=(data_object obj) {
    //     data = obj.data;
    //     hasData = obj.hasData.load(WEAK_ORDER);
    //     return *this;
    // }

//     bool isValid(){
//         return hasData;
//     }
//     int getData(){
//         return data;
//     } 
    

// };

using data = int;
using atomic_data = std::atomic<data>;
const data empty_data_val = -1;
const std::atomic<data> empty_data_val_atomic = empty_data_val;

// using atomic_object = std::atomic<data_object>;
// data_object *null_object = new data_object();
// atomic_object *atomic_null_object = (atomic_object*)null_object;
// atomic_object *NULL_Data_Object = (atomic_object*) new data_object();

#endif