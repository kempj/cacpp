#include <iostream>

#define GASNET_PAR 1
#include "gasnet.h"

using std::cout;
using std::endl;

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

int this_image(){
    return gasnet_mynode();;
}
int num_images(){
    return gasnet_nodes();
}

template<typename T, int NumDims>
class coref {
    public:
        coref(T *address, int id, int sz) {
            node_id = id;
            size = sz;
            data = address;
            //sub_corefs = new coref<T,NumDims-1>*[size]();
        }
        //copy:
        coref(const coref<T,NumDims>& other){
            data = other.data;
            node_id = other.node_id;
            size = other.size;
        }
        //move:
        coref(coref<T,NumDims>&& other) {
            data = other.data;
            size = other.size;
            node_id = other.node_id;
            //do I need to get rid of anything else?
        }
        //assignment:
        coref<T,NumDims>& operator=(coref<T,NumDims> other){
            data = other.data;
            //size should already be the same
            //if(other.node_id == this_image())
            //if(node_id == other.node_id)
            return *this;
        }
        coref<T,NumDims-1> operator[](int i){ 
            //return coref<T,NumDims-1>(std::forward<int*>(data + i), std::forward<int>(node_id));
            return coref<T,NumDims-1>(data + i, node_id);
        }
    private:
        T *data;
        int node_id;
        coref();
        int size;
        //coref<T,NumDims-1> **sub_corefs;
};
template<typename T>
class coref<T,0> {
    public:
        coref(T *address, int id) {
            data = address;
            node_id = id;
        }
        coref(const coref<T,0>& other){
            data = other.data;
            node_id = other.node_id;
        }
        coref(coref<T,0>&& other) {
            data = other.data;
            node_id = other.node_id;
        }
        coref<T,0>& operator=(coref<T,0> other){
            data = other.data;
            node_id = other.node_id;
            return *this;
        }
        operator T() {
            if(node_id != this_image()){
                gasnet_get(data, node_id, data, sizeof(T));
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
            return *this;  //I need to make and return a new coref object
        }
    private:
        coref();
        int node_id;
        T *data;
};

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size[NumDims]){
            int data_size = 1;
            for(int i=0; i < NumDims; i++){
                data_size *= size[i];
                extents[i] = size[i];
            }
            remote_init();
            T *local_data = new T[data_size];
            data = new coref<T,NumDims>(local_data, this_image(), extents[0]);
            remote_data[this_image()] = &(*data);
        }
        //coarray(int size[NumDims]): data(size){remote_init();}
        coref<T,NumDims>& operator()(int id){
            return *remote_data[id];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            return (*data)[i];
        }
        //const coref<T,NumDims-1>& operator[](int i){ 
        //    return (*data)[i];
        //}
    private:
        int extents[NumDims];
        void remote_init();
        coref<T, NumDims> *data;
        coref<T,NumDims> **remote_data;
};

template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init() {
    //This will eventually need to be replaced with a global array of 
    // pointers to the beginning of unallocated space
    remote_data = new coref<T,NumDims>*[num_images()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[num_images()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, num_images()));
    for(int i = 0; i < num_images(); i++) {
        if(this_image() != i) 
            remote_data[i] = new coref<T,NumDims>((T*)s[i].addr, i, extents[0]);
    }
    delete [] s;
}

