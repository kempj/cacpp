#include <iostream>
#include <array>

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
//int num_coarrays=0;
int64_t data_size=0;


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
        }
        coref(T *addr, int id, array<int,NumDims> sz): coref(addr, id, sz.data()) {}
        coref<T,NumDims-1> operator[](int i){ 
            int slice_size = 1;
            for(int i = 1; i < NumDims; i++) {
                slice_size *= size[i];
            }
            return coref<T,NumDims-1>(data + i*slice_size, node_id, &size[1]);
        }
    private:
        T *data;
        int node_id;
        coref();
        int size[NumDims];
};

template<typename T>
class coref <T,1>{
    public:
        coref(T *addr, int id, int sz[1]):node_id(id), data(addr), size(sz[0]){}
        coref(T *addr, int id, array<int,1> sz):node_id(id), data(addr), size(sz[0]){}
        coref<T,0> operator[](int i){ 
            return coref<T,0>(data + i, node_id);
        }
        coref<T,1>& operator=(coref<T,1> & other){
            //assert(size == other.size);
            std::copy(other.data, other.data + other.size, data);
        }
        coref<T,1>& operator=(T* const other){
            std::copy(other, other + size, data);
        }
        coref<T,1>& operator=(std::array<T,1> other){
            //assert(other.size() == size);
            std::copy(other.begin(), other.end(), data);
        }
        //operator T*() {}
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
        coarray(array<int,NumDims> const & size) : coarray(size.data()) {}
        coarray(const int size[NumDims]){
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
            //id = num_coarrays;
            //num_coarrays++;
            //T *local_data = new T[data_size];
            //data = new coref<T,NumDims>(local_data, this_image(), extents[0]);
            //remote_data[this_image()] = &(*data);
        }
        coref<T,NumDims>& operator()(int id){
            return *(remote_data[id]);
        }
        coref<T,NumDims-1> operator[](int i){ 
            return (*data)[i];
        }
        const coref<T,NumDims-1>& operator[](int i) const { 
            return (*data)[i];
        }
    private:
        int extents[NumDims];
        coref<T, NumDims> *data;
        coref<T,NumDims> **remote_data;
        //int id;
};
