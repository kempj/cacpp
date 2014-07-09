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
            if(this_image() != node_id) {
                data[i].addr = addr + i*sizeof(coref<T,NumDims-1>);//(&data[i] - &data[0]);
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
        //Might need to make extents a pointer for both templates, 
        // so the all the objects will be the same size on disk.
};
template<typename T>
class coref<T,0> {
    public:
        operator T(){
            return data;
        }
        /*
        coref<T,0>& operator=(coref<T,0> const& other){
            int tmp = 0;
            cout << "getting data" << endl;
            if(other.node_id != gasnet_mynode()){
                gasnet_get(&tmp, other.node_id, other.data, sizeof(T));
            } else {
                tmp = other.data;
            }
            if(node_id == gasnet_mynode()){
                cout << "Same Node" << endl;
                if(this != &other){
                    data = tmp;
                }
            } else {
                gasnet_put(data, node_id, &tmp, sizeof(T));
            }
            return *this;
        }*/
        //coref<T,0>& operator=(T const& other){
        coref<T,0>& operator=(T &other){
            coref<T,0> *tmp = (coref<T,0> *) addr;
            if(this_image() == node_id) {
                data = other;
            } else {
                cout << "Node " << this_image() << " putting data ("<< other 
                     <<") on node " << node_id  << " at " << &tmp->data << endl;
                gasnet_put(node_id, &tmp->data, &other, sizeof(T));
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

