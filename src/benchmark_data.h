#ifndef _BENCHMARK_DATA_
#define _BENCHMARK_DATA_
#include <stdio.h>

extern "C" {

struct benchmark_counters {
    int attempted_removes = 0; // gets incremented in the while loop of the remove method, so if a remove fails it will be attempted again and this counter will be incremented
    int successful_removes = 0; // everytime a thread returns an item from the remove method this gets incremented, except if the result is from a steal
    int attempted_steals = 0; // gets incremented in the while loop of the steal method, so if a steal is attempted and fails, another attempt is started and this counter will be incremented
    int successful_steals = 0;
    int items_added = 0;

};

}

void print_counters(benchmark_counters counters) {
    std::cout
        << "Attempted Removes: \t" << counters.attempted_removes << std::endl
        << "Successful Removes: \t" << counters.successful_removes << std::endl
        << "Attempted Steals: \t" << counters.attempted_steals << std::endl
        << "Successful Steals: \t" << counters.successful_steals << std::endl
        << "Items Added: \t\t" << counters.items_added << std::endl
        << std::endl;

}

extern "C" {

struct benchmark_result {
    float time;
    struct benchmark_counters reduced_counters;
};

}

void add_results(benchmark_result *res, benchmark_counters others) {
    res->reduced_counters.attempted_removes += others.attempted_removes;
    res->reduced_counters.successful_removes += others.successful_removes;
    res->reduced_counters.attempted_steals += others.attempted_steals;
    res->reduced_counters.successful_steals += others.successful_steals;
    res->reduced_counters.items_added += others.items_added;
}

extern "C" {

struct lock_benchmark_result {
    float time;
};

}
#endif