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
        coref(void *address, int id, int size) {
            //will need to detect whether or not it is the top level coref,
            // and allocate the whole array. 
            // would this be better to do from coarray?
            node_id = id;
            extents = size;
            addr = address;
            if(this_image() == id){
                //Only valid for one coarray of dim 1
                data = (coref<T,NumDims-1>*)address;
            } else {
                data = new coref<T, NumDims-1>[extents];
            }
        }
        coref<T,NumDims-1>& operator[](int i){ 
            //if I stored only one coref per dimension, I would need to 
            // create new corefs here and return them with their address 
            // and a reference to their data. This would require reference 
            // counting or using smart pointers.
            if(this_image() != node_id) {
                data[i].addr = (coref<T,NumDims-1> *)addr + i;
            }
            data[i].node_id = node_id;
            return data[i];
        }
        coref<T,NumDims-1> *data;
    private:
        int node_id;
        coref();
        int extents;
        void *addr;
};
template<typename T>
class coref<T,0> {
    public:
        operator T() {
            if(node_id != this_image()){
                gasnet_get(&data, node_id, &((coref<T,0> *) addr)->data, sizeof(T));
            }
            return data;
        }
        //Not sure if this would be helpful to add. Without it, 
        // the data is automaitically extracted using T() above, 
        // and then passed to operator=(T) below.
        // I would be duplicating the get and put logic.
        // This would be helpful for higher dimension arrays in the future;
        // being able to send an array from a remote machine to another remote machine
        // without having to copy it locally.
        /*
        coref<T,0>& operator=(coref<T,0> & other){
            if(this == & other)
                return *this;
            if(other.node_id == node_id) {
                if(node_id == this_image()) {
                    //local to local
                    data = other.data;
                } else {
                    //remote to itself
                    *this = T(other);//this can be optimized;
                }
            } else {
                if(node_id != this_image() && other.node_id != this_image()){
                    //remote to another remote
                    *this = T(other);//can be optimized
                } else if(node_id == this_image()) {
                    //local to remote
                    *this = T(other);
                } else {
                    //remote to local
                    other = T(*this);
                }
            }

        }*/

        //I need to make a new coref object and return it
        coref<T,0>& operator=(T const& other){
            data = other;
            if(this_image() != node_id) {
                gasnet_put(node_id, &((coref<T,0> *) addr)->data, &data, sizeof(T));
            }
            return *this;
        }
        int node_id;
        void *addr;
        T data;
    private:
        int extents;
};

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size):size(size){
            remote_init();
        }
        //coarray(int size[NumDims]): data(size){remote_init();}
        coref<T,NumDims>& operator()(int id){
            return *remote_data[id];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            return (*data)[i];
        }
    private:
        int size;
        void remote_init();
        coref<T, NumDims> *data;
        coref<T,NumDims> **remote_data;
};

template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init() {
    remote_data = new coref<T,NumDims>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
        remote_data[i] = new coref<T,NumDims>(s[i].addr, i, size);
        if(gasnet_mynode() == i) 
            data = remote_data[i] ;
    }
    delete [] s;
}

