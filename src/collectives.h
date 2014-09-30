#include "coarray.h"

template<typename T, int NumDims>
void gather(local_array<T> result, coarray<T,NumDims> data){
    for(size_t i =0; i < num_images(); i++) {
        size_t offset = data.size();
        for(int j = 0; j < data.size(); j++) {
            result[offset + j] = data(i)[j];
        }
    }
    //size(local_array) * num_images() = size(coarray)
};
/*
void scatter(local_array data, coarray result){
    //size(local_array) = size(coarray)* num_images()
};

void collect(local_array scratch, coarray data){
    //size(local_array) = size(coarray)
};
*/  
