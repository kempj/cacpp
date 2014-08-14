#include <iostream>
#include <cstdarg>
#include <cassert>
#include <atomic>

#include "runtime.h"
#include "coref.h"
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
    return RT.get_num_images()
}

void sync_all() {
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    int status = gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    if(GASNET_OK != status)
        cout << "error while syncing all" << endl;
}

void sync_images(int *image_list, size_t size) {
    num_waiting_images += size;
    for(int i = 0; i < size; i++) {
        gasnet_AMRequestShort1(image_list[i], 128, -1);
    }
    GASNET_BLOCKUNTIL(num_waiting_images == 0);
}


template<typename T, int NumDims>
class coarray {
    public:
        coarray(int *D, int cosize, int *C) : coarray(   dims(D,NumDims), 
                                                       codims(C, cosize) ) {}
        coarray(dims size, codims cosize) {
            //throw if not codim1 * codim2 *... = num_images
            rt_id = rt.coarray_setup(dims.D, codims.D);
            node_id = RT.get_image_id();
        }
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            //TODO: get this working
        }
        coref<T,NumDims>& operator()(){
            return *(remote_data[image_num]);
        }
        coref<T,NumDims>& operator()(int id){
            return *(remote_data[id]);
        }
        /*
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
        coref<T,NumDims-1> operator[](int i) const { 
            return (*data)[i];
        }
    private:
        uint64_t rt_id;
        std::vector<uint64_t> start_coords;
        int node_id;
};


