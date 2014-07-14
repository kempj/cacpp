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
        coref(T *address, int id, int sz[NumDims]):node_id(id), data(address) {
            std::copy(sz, sz + NumDims, size);
        }
        coref<T,NumDims-1> operator[](int i){ 
            int newsize[NumDims-1];
            std::copy(size+1, size + NumDims, newsize);
            return coref<T,NumDims-1>(data + i*size[1], node_id, newsize);
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
        coref(T *address, int id, int sz[1]):node_id(id), data(address), size(sz[0]){}
        coref<T,0> operator[](int i){ 
            return coref<T,0>(data + i, node_id);
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
        coref<T,0>& operator=(coref<T,0> & other){
            *this = T(other.data);
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
        coarray(int size[NumDims]){
            int data_size = 1;
            for(int i=0; i < NumDims; i++){
                data_size *= size[i];
                extents[i] = size[i];
            }
            remote_init();
            data = remote_data[this_image()];
            //T *local_data = new T[data_size];
            //data = new coref<T,NumDims>(local_data, this_image(), extents[0]);
            //remote_data[this_image()] = &(*data);
        }
        //coarray(int size[NumDims]): data(size){remote_init();}
        coref<T,NumDims>& operator()(int id){
            return *remote_data[id];
        }
        //FIXME:if this isn't a reference, then I can't assign to it and have the results be persistent.
        coref<T,NumDims-1> operator[](int i){ 
            return (*data)[i];
        }
        const coref<T,NumDims-1>& operator[](int i) const { 
            return (*data)[i];
        }
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
        remote_data[i] = new coref<T,NumDims>((T*)s[i].addr, i, extents);
    }
    delete [] s;
}

