#include <iostream>
#include <cstdarg>
#include <cassert>

#include "runtime.h"
//#include "coref.h"
#include "dims.h"
#include "codims.h"

using std::atomic;
using std::cout;
using std::endl;

coarray_runtime RT;//do I need to call () here?

int this_image(){
    return RT.get_image_id();
}
int num_images(){
    return RT.get_num_images();
}

void sync_all() {
    RT.barrier();
}

void sync_images(int *image_list, size_t size) {
    RT.sync_images(image_list, size);
}

template<typename T, int NumDims>
class coarray {
    public:
        coarray(int *D, int cosize, int *C) : coarray( dims(D,NumDims), 
                                                       codims(C, cosize) ) {}
        coarray(dims size) : coarray(size, codims()) {}
        coarray(dims size, codims cosize) {
            data.rt_id = RT.coarray_setup(size.D, cosize.D, sizeof(T));
            data.node_id = RT.get_image_id();
        }
        coarray(int id, location_data parent_data) : data(parent_data) {
            data.node_id = id;
        }
        coarray(location_data parent_data, uint64_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            T *lhs_address = ((T*)RT.get_address(data));
            T *rhs_address = ((T*)RT.get_address(other.data)); 
            if(RT.is_local(data) && RT.is_local(other.data)){
                *lhs_address = *rhs_address;
            } else if(!RT.is_local(other.data) && RT.is_local(data)) {
                RT.get(lhs_address, other.data);
            } else if(!RT.is_local(data) && RT.is_local(other.data)) {
                RT.put(rhs_address, data);
            } else if(!RT.is_local(data) && !RT.is_local(other.data)) {
                T *tmp = new T[RT.size(data)];
                RT.get(tmp, other.data);
                RT.put(tmp, data);
                delete[] tmp;
            } //else throw?
            return *this;
        }
        coarray<T,NumDims>& operator()(){
            return *this;
        }
        coarray<T,NumDims> operator()(int id){
            return coarray(id, data);
        }
        coarray<T,NumDims>& operator()(int first, int second, ...){
            va_list args;
            va_start(args, second);
            std::vector<int> index;
            index.push_back(first);
            index.push_back(second);
            uint64_t cosize = RT.handles[data.rt_id].codims.size();
            for(int i = 2; i < cosize; i++){
                index.push_back(va_arg(args, int));
            }
            va_end(args);

            int current_index = index.back();
            for(int i = index.size()-1; i > 0; i--){
                current_index += index[i-1] * codims[i];
            }
            return coarray(current_index, *this);
        }
        coarray<T,NumDims-1> operator[](uint64_t i) { 
            return coarray<T,NumDims-1>(data, i);
        }
        T* get_data() {
            return (T*)RT.get_address(data);
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
        coarray(location_data parent_data, uint64_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        const coarray<T,0>& operator=(const coarray<T,0> &other) {
            T *data_addr = (T*)RT.get_address(data);

            T tmp;
            if(!RT.is_local(data)){
                //data_addr is not a valid address on this node; a copy is needed
                data_addr= &tmp;
            } 
            *data_addr = T(other);

            if(!RT.is_local(data)) {
                RT.put((*void)data_addr, data);
            }
            return *this;
        }
        const coarray<T,0>& operator=(const T &other) const {
            if(RT.is_local(data)){
                *((T*) (RT.get_address(data))) = other;
            } else {
                T tmp = other;
                RT.put(&tmp, data);
            }
            return *this;
        }
        operator T() const {
            T tmp;
            if(RT.is_local(data)){
                tmp = *((T*) (RT.get_address(data)));
            } else {
                RT.get(&tmp, data);
            }
            return tmp;
        }
    private:
        coarray();
        location_data data;
};

