#include <iostream>
#include <array>
#include <cstdarg>
#include <cassert>

#include "coref.h"
#include "dims.h"
#include "codims.h"

#define GASNET_PAR 1
#include "gasnet.h"

using std::cout;
using std::endl;
using std::array;

/* Macro to check return codes and terminate with useful message. */
#define GASNET_SAFE(fncall) do {                                    \
    int _retval;                                                    \
    if ((_retval = fncall) != GASNET_OK)                            \
    {                                                               \
        fprintf(stderr, "ERROR calling: %s\n"                       \
                        " at: %s:%i\n"                              \
                        " error: %s (%s)\n",                        \
                        #fncall, __FILE__, __LINE__,                \
              gasnet_ErrorName(_retval), gasnet_ErrorDesc(_retval));\
        fflush(stderr);                                             \
        gasnet_exit(_retval);                                       \
    }                                                               \
} while(0)


gasnet_seginfo_t *segment_info;
int image_num=-1;
int64_t data_size=0;//Needed globally to keep track of the beginning of each new coarray

int this_image(){
    return gasnet_mynode();;
}
int num_images(){
    return gasnet_nodes();
}

void sync_all() {
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    int status = gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    if(GASNET_OK != status)
        cout << "error while syncing all" << endl;
}

void remote_init(){
    if(!segment_info) {
        segment_info =  new gasnet_seginfo_t[num_images()];
        int status = gasnet_getSegmentInfo(segment_info, num_images());
        if( GASNET_OK != status)
            cout << "failed to get segment info" << endl;
    }
}

template<typename T, int NumDims>
class coarray {
    public:
        coarray(dims size, codims cosize) : coarray(size.D) {
            codims = cosize.D;
            //assert codim1 * codim2 *... = num_images
        }
        coarray(array<int,NumDims> const & dim, array<int,NumDims> const & codim) : coarray(dim.data()) {
            codims = codim;
            //std::copy(codim.begin(), codim.end(), 
        }
        coarray(const int dim[NumDims], const int codim[NumDims]) : coarray(dim){
            codims = codim
        }
        coarray(dims size) : coarray(size.D) {
        }
        coarray(array<int,NumDims> const & size) : coarray(size.data()) {
        }
        coarray(const int size[NumDims]){
            image_num=this_image();
            if(codims.size() == 0)
                codims.push_back(1);
            int local_size=1;
            for(int i=0; i < NumDims; i++){
                local_size*= size[i];
                extents[i] = size[i];
            }
            remote_init();
            remote_data = new coref<T,NumDims>*[num_images()];
            for(int i = 0; i < num_images(); i++) {
                remote_data[i] = new coref<T,NumDims>(((T*)segment_info[i].addr + data_size), i, extents);
            }
            data = remote_data[image_num];
            data_size += (local_size * sizeof(T));
        }
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            //TODO: get this working
            //if they are both on same node (and not same array) 
            // then just swap pointers.
            //Same array, different nodes?
        }
        coref<T,NumDims>& operator()(){
            return *(remote_data[image_num]);
        }
        coref<T,NumDims>& operator()(int id){
            return *(remote_data[id]);
        }
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
        coref<T,NumDims-1> operator[](int i){ 
            return (*data)[i];
        }
        const coref<T,NumDims-1>& operator[](int i) const { 
            return (*data)[i];
        }
        coref<T, NumDims-1> begin() {
            return data->begin();
        }
        coref<T, NumDims-1> end() { 
            return data->end();
        }
    private:
        std::vector<int> codims;
        int extents[NumDims];
        coref<T, NumDims> *data;
        coref<T,NumDims> **remote_data;
};
