#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdbool.h>
#include <omp.h>

class tObject
{



public:
    int x;
    int *arr;


    tObject(int size){
        x = -1;
        arr = new int[10];
        for (int i = 0; i < size; i++)
        {
            arr[i] = -1;
        }
        
        
    }
    
    void setX(int val){
        #pragma omp atomic write
        this->x = val;
    }
    int getX(){
        return x;
    }
};