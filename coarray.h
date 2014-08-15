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
        coarray(int *D, int cosize, int *C) : coarray(   dims(D,NumDims), 
                                                       codims(C, cosize) ) {}
        coarray(dims size, codims cosize) {
            data.rt_id = RT.coarray_setup(size.D, cosize.D, sizeof(T));
            data.node_id = RT.get_image_id();
        }
        coarray(location_data parent_data, uint64_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            return *this;//TODO
        }
        coarray<T,NumDims>& operator()(){
            return *this;
        }
        coarray<T,NumDims> operator()(int id){
            return coarray(data, id);
        }
        coarray<T,NumDims-1> operator[](uint64_t i) { 
            return coarray<T,NumDims-1>(data, i);
        }
    private:
        coarray();
        location_data data;
};

template<typename T>
class coarray<T,0> {
    public:
        coarray(location_data parent_data, uint64_t i) : data(parent_data) {
            data.start_coords.push_back(i);
        }
        const coarray<T,0>& operator=(const coarray<T,0> &other) {
            //T *src =  (T*)RT.get_address(other.data);
            T *data_addr = (T*)RT.get_address(data);

            T tmp;
            if(!RT.is_local_node(data.node_id)){
                //data_addr is not a valid address on this node,
                // so a temporary copy is needed
                data_addr= &tmp;
            } 
            *data_addr = T(other);

            if(!RT.is_local_node(data.node_id)) {
                RT.put((*void)data_addr, data);//get_address(data), data.node_id, size);
            }
            return *this;
        }
        operator T() const {
            T tmp;
            if(RT.is_local_node(data.node_id)){
                tmp = *((T*) (RT.get_address(data)));
            } else {
                RT.get(&tmp, data);
            }
            return tmp;
        }
        const coarray<T,0>& operator=(const T &other) const {
            //TODO
            /*
            T tmp = other;
            if(image_num != node_id) {
                gasnet_put(node_id, data, &tmp, sizeof(T));
            } else {
                *data = other;
            }*/
            return *this;
        }
    private:
        coarray();
        location_data data;
};

        /*
        //TODO
        coref<T,NumDims>& operator()(int first, int second, ...){
            va_list args;
            va_start(args, second);
            std::vector<int> index;
            index.push_back(first);
            index.push_back(second);
            for(int i = 2; i < codims.size(); i++){
                index.push_back(va_arg(args, int));
            }
            va_end(args);

            int current_index = index.back();
            for(int i = index.size()-1; i > 0; i--){
                current_index += index[i-1] * codims[i];
            }
            return *(remote_data[current_index]);
        }
        */
