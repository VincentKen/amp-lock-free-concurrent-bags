#include <stdio.h>

struct benchmark_counters {
    int attempted_removes = 0; // gets incremented in the while loop of the remove method, so if a remove fails it will be attempted again and this counter will be incremented
    int successful_removes = 0; // everytime a thread returns an item from the remove method this gets incremented, except if the result is from a steal
    int attempted_steals = 0; // gets incremented in the while loop of the steal method, so if a steal is attempted and fails, another attempt is started and this counter will be incremented
    int successful_steals = 0;
    int items_added = 0;

    void print() {
        std::cout
            << "Attempted Removes: \t" << attempted_removes << std::endl
            << "Successful Removes: \t" << successful_removes << std::endl
            << "Attempted Steals: \t" << attempted_steals << std::endl
            << "Successful Steals: \t" << successful_steals << std::endl
            << "Items Added: \t\t" << items_added << std::endl
            << std::endl;

    }
};
struct benchmark_result {
    float time;
    struct benchmark_counters reduced_counters;

    void add_results(benchmark_counters others) {
        reduced_counters.attempted_removes += others.attempted_removes;
        reduced_counters.successful_removes += others.successful_removes;
        reduced_counters.attempted_steals += others.attempted_steals;
        reduced_counters.successful_steals += others.successful_steals;
        reduced_counters.items_added += others.items_added;
    }
};