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


template<typename T, int NumDims>
class coref {
    public:
        /*coref(int size = 1) {
            for(int i = 0; i < NumDims; i++)
                extents[i] = size;
            //data = new coref<T,NumDims-1>[size];
        }*/
        coref(void *address, int id) {
            //TODO: this is ugly. Double check for correctness
            data = (coref<T,NumDims-1>*)address;
            node_id = id;
        }
        coref<T,NumDims-1>& operator[](int i){ 
            data[i].node_id = node_id;
            return data[i];
        }
        int node_id;
    private:
        coref<T,NumDims-1> *data;
        int extents[NumDims];
        //void *addr;
        //bool is_local;
        //Might need to make extents a pointer for both templates, 
        // so the all the objects will be the same size on disk.
};
template<typename T>
class coref<T,0> {
    public:
        operator T(){
            return data;
        }
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
        }
        coref<T,0>& operator=(T const& other){
            cout << "other assignment" << endl;
            data = other;
            return *this;
        }
        int node_id;
    private:
        T data;
};

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size){
            remote_init();
        }
        //coarray(int size[NumDims]): data(size){remote_init();}
        coref<T,NumDims>& operator()(int id){
            return *remote_data[id];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            //if(gasnet_mynode() == node_id )
            return (*data)[i];
        }
    private:
        void remote_init();
        coref<T, NumDims> *data;
        coref<T,NumDims> **remote_data;
        //void* address;
        //int node_id;
};

template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init() {
    remote_data = new coref<T,NumDims>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
            remote_data[i] = new coref<T,NumDims>(s[i].addr, i);
        if(gasnet_mynode() == i) 
            data = remote_data[i] ;
    }
    delete [] s;
}

int this_image(){
    return gasnet_mynode();;
}

int num_images(){
    return gasnet_nodes();
}

