#ifndef data_object_h
#define data_object_h

#include <atomic>


#define WEAK_ORDER std::memory_order_relaxed // std::memory_order_relaxed is waaaay too long to type out every time

using data = int;
using atomic_data = std::atomic<data>;
const data empty_data_val = -1;
const std::atomic<data> empty_data_val_atomic = empty_data_val;


#endif