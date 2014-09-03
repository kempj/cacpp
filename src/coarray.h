#include <iostream>
#include <cstdarg>
#include <cassert>
#include <memory>

#include "runtime.h"
#include "dims.h"
#include "codims.h"
#include "range.h"
//#include "local_array.h"

#include <type_traits>

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

template<typename T, size_t NumDims, size_t MaxDim = NumDims>
class coarray {
    public:
        std::array<size_t,MaxDim> first_coord{};
        std::array<size_t,MaxDim> last_coord{};
        size_t rt_id;
        size_t node_id;

        coarray(int *D, int cosize, int *C) : coarray( dims(D,NumDims), 
                                                       codims(C, cosize) ) {}
        coarray(dims size) : coarray(size, codims()) {}
        coarray(dims size, codims cosize) {
            rt_id = RT->coarray_setup(size.D, cosize.D, sizeof(T));
            node_id = RT->get_image_id();
            std::copy(&(size.D[0]), &(size.D[0]) + NumDims, &last_coord[0]);
        }
        coarray( std::array<size_t, MaxDim> fc, std::array<size_t,MaxDim> lc,
                 size_t rt, int node)
                : first_coord(fc), last_coord(lc), rt_id(rt), node_id(node) {}
        
        coarray<T,NumDims,MaxDim>& operator=(coarray<T,NumDims,MaxDim> &other) {
            T *dest = ((T*)RT->get_address( first_coord, rt_id, node_id ));
            T *src = ((T*)RT->get_address( other.first_coord, other.rt_id, other.node_id)); 
            size_t data_size = other.size()*sizeof(T);

            if(this->is_local() && other.is_local()){
                std::copy(src, src + size(), dest);
            } else if(!other.is_local() && this->is_local()) {
                RT->get(src, dest, other.node_id, data_size);
            } else if(!this->is_local() && other.is_local()) {
                RT->put(src, dest, node_id, data_size);
            } else if(!this->is_local() && !other.is_local()) {
                T *tmp = new T[size()];
                RT->get(src, tmp, other.node_id, data_size);
                RT->put(tmp, dest, node_id, data_size);
                delete[] tmp;
            }
            return *this;
        }
        const coarray<T,0,MaxDim>& operator=(const T &other) const {
            static_assert(NumDims == 0, "Trying to assign type T to array");
            T* address = (T*) (RT->get_address(rt_id, node_id));
            if(this->is_local()){
                *address = other;
            } else {
                T tmp = other;
                RT->put(&tmp, address, node_id, sizeof(T) );
            }
            return *this;
        }
        operator T() const {
            static_assert(NumDims == 0, "Trying to convert array to type T");
            T tmp;
            T* address = (T*) (RT->get_address(rt_id, node_id));
            if(this->is_local()){
                tmp = *address;
            } else {
                RT->get(address, &tmp, node_id, sizeof(T));
            }
            return tmp;
        }
        typedef coarray<T,(NumDims-1)*(NumDims>=0),MaxDim> subarray_type;

        subarray_type operator[](range R) {
            subarray_type tmp(first_coord, last_coord, rt_id, node_id); 
            const int index = MaxDim-NumDims;
            if(R.all){
                tmp.first_coord[index] = 0;
                tmp.last_coord[index] = RT->get_dim(rt_id, index);
            } else {
                tmp.first_coord[index] = R.start;
                tmp.last_coord[index] = R.end;
                //throw if start or end are out of range?
            }
            return tmp;
        }
        subarray_type operator[](size_t i) { 
            static_assert(NumDims >= 0, "cannot index (co)scalar");
            first_coord[MaxDim-NumDims] = i;
            last_coord[MaxDim-NumDims] = i;
            return subarray_type(first_coord, last_coord, rt_id, node_id);
        }
        T* begin() {
            return (T*)(RT->get_address(first_coord, rt_id, node_id));
        }
        T* end() {
            return (T*)(RT->get_address(last_coord, rt_id, node_id));
        }
        size_t size() {
            int val = 1;
            for(int i =0; i < MaxDim; i++){
                val *= (last_coord[i] - first_coord[i]);
            }
            return val;
        }
        bool is_local() const{
            return node_id == RT->get_image_id();
        }
        void copy_to(T* addr){
            //This doesn't work if the data is noncontiguous. 
            //Should this just collect all the data into an array of size size()?
            //or throw if the data is noncontiguous?
            if(!is_local()){
                RT->get(begin(), addr, node_id, size()*sizeof(T));
            } else {
                std::copy(begin(), end(), addr);
            }
        }
        
        coarray<T,NumDims,MaxDim>& operator()(){
            return *this;
        }
        coarray<T,NumDims,MaxDim> operator()(int id){
            coarray<T,NumDims,MaxDim> tmp(*this); 
            tmp.node_id = id;
            return tmp;
        }
        coarray<T,NumDims,MaxDim>& operator()(int first, int second, ...){
            va_list args;
            va_start(args, second);
            std::vector<int> index;
            index.push_back(first);
            index.push_back(second);
            vector<size_t> codims  = RT->get_codims(rt_id);
            for(size_t i = 2; i < codims.size(); i++){
                index.push_back(va_arg(args, int));
            }
            va_end(args);

            int current_index = index.back();
            for(int i = index.size()-1; i > 0; i--){
                current_index += index[i-1] * codims[i];
            }
            return (*this)(current_index);
        }
    private:
        coarray();
};

template<typename T>
class local_array {
    public:
        local_array(size_t size): _size(size) {
            data = new T[size];
        }
        ~local_array() {
            delete[] data;
        }

        template<size_t NumDims, size_t MaxDim = NumDims>
        local_array<T>& operator=(coarray<T, NumDims, MaxDim> orig){
            if(!data){
                data = new T[orig.size()];
            }
            if(!orig.is_local()){
                size_t count[NumDims];
                for(int i = 0; i < NumDims; i++) {
                    count[i] = (orig.last_coord[i] - orig.first_coord[i]) * sizeof(T);
                }
                //RT->get(orig.begin(), data, orig.node_id, orig.size()*sizeof(T));
                RT->gets(orig.begin(), data, NumDims, orig.node_id, count, orig.rt_id);
            } else {
                std::copy(orig.begin(), orig.end(), data);
            }
        }
        T operator[](size_t idx) {
            return data[idx];
        }
    private:
        //Do I benefit from forcing the user to specify the size
        // by making the default constructor private?
        //TODO: use smart pointer?
        T* data;
        size_t _size;
};

