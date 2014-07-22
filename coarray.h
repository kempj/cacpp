#include <iostream>
#include <array>
#include <cstdarg>
#include <cassert>

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
class coref {
    public:
        coref(T *address, int id, int sz[NumDims]):node_id(id), data(address) {
            std::copy(sz, sz + NumDims, &size[0]);
            total_size = 1;//TMP
            slice_size = 1;
            for(int i = 0; i < NumDims; i++) {
                total_size *= size[i];
                if(i > 0) {
                    slice_size *= size[i];
                }
            }
        }
        coref(T *addr, int id, array<int,NumDims> sz): coref(addr, id, sz.data()) {}
        coref<T,NumDims-1> operator[](int i){ 
            return coref<T,NumDims-1>(data + i*slice_size, node_id, &size[1]);
        }
        coref<T,NumDims> operator=(coref<T,NumDims> other){
            assert(size == other.size);
            std::copy(other.data, other.data + other.total_size, data);
            return *this;
        }
        coref<T, NumDims-1> begin() {
            return coref<T,NumDims-1>(data, node_id, &size[1]);
        }
        coref<T, NumDims-1> end() { 
            return coref<T,NumDims-1>(data + (size[0]-1)*slice_size, node_id, &size[1]);
        }
        bool operator!=(const coref<T,NumDims> other) const {
            return !( (data == other.data) && (node_id == other.node_id) );
        }
        const coref<T,NumDims> operator++() {
            data += slice_size;
            return *this;
        }
        coref<T,1> operator*() {
            return *this;
        }
    private:
        T *data;
        int node_id;
        coref();
        int size[NumDims];
        int total_size;
        int slice_size;
};

template<typename T>
class coref <T,1> {
    public:
        coref(T *addr, int id, int sz):node_id(id), data(addr), size(sz){}
        coref(T *addr, int id, int sz[1]):node_id(id), data(addr), size(sz[0]){}

        coref(T *addr, int id, array<int,1> sz):node_id(id), data(addr), size(sz[0]){}

        coref<T,0> operator[](int i){ 
            return coref<T,0>(data + i, node_id);
        }
        coref<T,1> operator=(coref<T,1> other){
            assert(size == other.size);
            
            if((node_id == this_image()) && (other.node_id == this_image())) {
                std::copy(other.data, other.data + other.size, data);
                return *this;
            }

            T *tmp_data = data;
            if( other.node_id != this_image() ) {//rhs is remote
                if( node_id != this_image() ) {//both sides are remote
                    tmp_data = new T[size];
                }
                gasnet_get_bulk(tmp_data, other.node_id, other.data, size * sizeof(T));
            }
            if( node_id != this_image() ) {//lhs is remote
                //send data
                gasnet_put_bulk(node_id, data, tmp_data, size*sizeof(T));
            }
            if( tmp_data != data)
                delete[] tmp_data;
            return *this;
        }

        coref<T,1>& operator=(T* const other){
            assert(false);
            if( node_id != this_image() ) {
            }
            std::copy(other, other + size, data);
            return *this;
        }
        coref<T,1>& operator=(std::array<T,1> other){
            assert(other.size() == size);
            std::copy(other.begin(), other.end(), data);
            return *this;
        }
        T* begin() {
            return &data[0];
        }
        T* end() {
            return &data[size-1];
        }
        bool operator!=(const coref<T,1> other) const {
            return !( (data == other.data) && (node_id == other.node_id) );
        }
        const coref<T,1> operator++() {
            data += size;
            return *this;
        }
        coref<T,1> operator*() {
            return *this;
        }
    private:
        T *data;
        int node_id;
        coref();
        int size;
};

template<typename T>
class coref<T,0> {
    public:
        coref(T *address, int id) {
            data = address;
            node_id = id;
        }
        //TODO: define other operators, like += and >>
        coref<T,0>& operator=(coref<T,0> & other){
            this->data = T(other);
            return *this;
        }
        operator T() {
            T tmp;
            if(node_id != this_image()){
                gasnet_get(&tmp, node_id, data, sizeof(T));
                return tmp;
            }
            return *data;
        }
        coref<T,0>& operator=(T const& other){
            T tmp = other;
            if(this_image() != node_id) {
                gasnet_put(node_id, data, &tmp, sizeof(T));
            } else {
                *data = other;
            }
            return *this;
        }
    private:
        coref();
        int node_id;
        T *data;
};

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(dims size, codims cosize) : coarray(size.D) {
            codims = cosize.D;
            //assert codim1 * codim2 *... = num_images
        }

        coarray(dims size) : coarray(size.D) {}

        coarray(array<int,NumDims> const & size) : coarray(size.data()) {}

        coarray(const int size[NumDims]){
            if(codims.size() == 0)
                codims.push_back(1);

            int local_size= 1;
            for(int i=0; i < NumDims; i++){
                local_size*= size[i];
                extents[i] = size[i];
            }
            remote_init();
            remote_data = new coref<T,NumDims>*[num_images()];
            for(int i = 0; i < num_images(); i++) {
                remote_data[i] = new coref<T,NumDims>(((T*)segment_info[i].addr + data_size), i, extents);
            }
            data = remote_data[this_image()];
            data_size += (local_size * sizeof(T));
        }

        //coarray<T,NumDims>& operator=(coref<T,NumDims> &other) {
        coarray<T,NumDims>& operator=(coarray<T,NumDims> &other) {
            //if they are both on same node (and not same array) 
            // then just swap pointers.
            //Same array, different nodes?
        }
        coref<T,NumDims>& operator()(){
            return *(remote_data[this_image()]);
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
