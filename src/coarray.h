#include <iostream>
#include <cstdarg>
#include <cassert>
#include <memory>

#include "runtime.h"
#include "dims.h"
#include "codims.h"
#include "range.h"

using std::atomic;
using std::cout;
using std::endl;

std::shared_ptr<coarray_runtime> RT;

void coarray_init( size_t segsize = 4*1024*1024, int argc = 0, char **argv = NULL ){
    if(argc == 0){
        char *tmp = (char*)"coarrayRT";
        argv = &tmp;
        argc = 1;
    }
    segsize -= segsize % 4*1024;
    RT.reset(new coarray_runtime(segsize, argc, argv));
}

void coarray_exit() {
    RT->barrier();
    gasnet_exit(RT->retval);
}

int this_image(){
    return RT->get_image_id();
}
int num_images(){
    return RT->get_num_images();
}

void sync_all() {
    RT->barrier();
}

void sync_images(int *image_list, size_t size) {
    RT->sync_images(image_list, size);
}

template<typename T, int NumDims>
class coarray {
    public:
        //Constructors:
        coarray(int *D, int cosize, int *C) : coarray( dims(D,NumDims), 
                                                       codims(C, cosize) ) {}
        coarray(dims size) : coarray(size, codims()) {}
        coarray(dims size, codims cosize) {
            data.rt_id = RT->coarray_setup(size.D, cosize.D, sizeof(T));
            data.node_id = RT->get_image_id();
        }
        coarray(location_data parent_data) : data(parent_data) {
        }
        coarray(location_data parent_data, size_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        //--------------
        //TODO: check for extents, and do strided gets and puts, and local copies.
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            T *lhs = ((T*)RT->get_address(data));
            T *rhs = ((T*)RT->get_address(other.data)); 
            if(this->is_local() && other.is_local()){
                *lhs = *rhs;
            } else if(!other.is_local() && this->is_local()) {
                RT->get(lhs, other.data);
            } else if(!this->is_local() && other.is_local()) {
                RT->put(rhs, data);
            } else if(!this->is_local() && !other.is_local()) {
                T *tmp = new T[RT->size(data)];
                RT->get(tmp, other.data);
                RT->put(tmp, data);
                delete[] tmp;
            } //else throw?
            return *this;
        }

        coarray<T,NumDims> operator[](range R) {
            coarray<T, NumDims> tmp(data);
            size_t start, end;

            //assert(tmp.data.start_coords.size() == NumDims-1);
            
            if(R.all){
                start = 0;
                end = RT->get_dim(data, NumDims-1);
            } else {
                start = R.start;
                end = R.end;
            }

            if(tmp.data.start_coords.size() > 0) {
                tmp.data.start_coords[tmp.data.start_coords.size() - 1] = start;
            } else {
                tmp.data.start_coords.push_back(start);
            }
            tmp.data.extents.push_back(end);
        }

        coarray<T,NumDims-1> operator[](size_t i) { 
            return coarray<T,NumDims-1>(data, i);
        }
        operator T*() {
            return (T*)RT->get_address(data);
        }
        T* begin() {
            return (T*)RT->get_address(data);
        }
        T* end() {
            return (T*)RT->get_address(data) + RT->size(data);
        }
        size_t size() {
            return RT->size(data);
        }
        bool is_local() {
            return RT->is_local(data);
        }
        void copy_to(T* addr){
            if(!is_local()){
                RT->get(addr, data);
            } else {
                data = std::copy(begin(), end(), data);
            }
        }
        
        coarray<T,NumDims>& operator()(){
            return *this;
        }
        coarray<T,NumDims> operator()(int id){
            coarray<T,NumDims> tmp(data); 
            tmp.data.node_id = id;
            return tmp;
        }
        coarray<T,NumDims>& operator()(int first, int second, ...){
            va_list args;
            va_start(args, second);
            std::vector<int> index;
            index.push_back(first);
            index.push_back(second);
            vector<size_t> codims  = RT->get_codims(data);
            for(size_t i = 2; i < codims.size(); i++){
                index.push_back(va_arg(args, int));
            }
            va_end(args);

            int current_index = index.back();
            for(int i = index.size()-1; i > 0; i--){
                current_index += index[i-1] * codims[i];
            }
            return coarray(current_index, *this);
        }
    private:
        coarray();
        location_data data;
};

template<typename T>
class coarray<T,0> {
    public:
        //TODO, decide how to do co-scalars
        //coarray(dims size, codims cosize) {
        //    data.rt_id = RT.coarray_setup(size.D, cosize.D, sizeof(T));
        //    data.node_id = RT.get_image_id();
        //}
        coarray(int id, location_data parent_data) : data(parent_data) {
            data.node_id = id;
        }
        coarray(location_data parent_data, size_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        const coarray<T,0>& operator=(const coarray<T,0> &other) {
            T *data_addr = (T*)RT->get_address(data);
            T tmp;
            if(!RT->is_local(data)){
                //data_addr is not a valid address on this node; a copy is needed
                data_addr= &tmp;
            } 
            *data_addr = T(other);

            if(!RT->is_local(data)) {
                RT->put((void*)data_addr, data);
            }
            return *this;
        }
        const coarray<T,0>& operator=(const T &other) const {
            if(RT->is_local(data)){
                *((T*) (RT->get_address(data))) = other;
            } else {
                T tmp = other;
                RT->put(&tmp, data);
            }
            return *this;
        }
        operator T() const {
            T tmp;
            if(RT->is_local(data)){
                tmp = *((T*) (RT->get_address(data)));
            } else {
                RT->get(&tmp, data);
            }
            return tmp;
        }
    private:
        coarray();
        location_data data;
};

