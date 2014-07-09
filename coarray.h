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

//Do I need a specialization for NumDims = 1, or any other dimensions?
//What about a coarray of a scalar or single object?

//This will need to be overloaded for NumDims=1
template<typename T, int NumDims = 1>
class coref {
    public:
        coref(int size = 1){
            extents[0] = size;
            data = new T[size];
        }
        //coref<T,NumDims-1>& operator[](int i){ 
        T& operator[](int i){ 
            return data[i];
        }
    private:
        T* data;
        int extents[NumDims];
};
/*
template<typename T>
T&
coref<T,1>::operator[](int id) {
    return data[i];
}*/

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        //coarray(int size[NumDims]): data(size){
        //    remote_init();
        coarray(int size): data(size){
            remote_init();
        }
        coarray(gasnet_seginfo_t s){
            node_info = s;
        }
        coarray<T,NumDims>& operator()(int id){
            return *remote_coarrays[id];
        }
        T& operator[](int i){ 
            return data[i];
        }
    private:
        void remote_init();
        coref<T, NumDims> data;
        //const static int dim = NumDims;
        coarray<T,NumDims> **remote_coarrays;
        gasnet_seginfo_t node_info;
};


template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init() {
    remote_coarrays = new coarray<T,NumDims>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];//might want to change to smart pointer, and share with sub arrays
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
        if(gasnet_mynode() == i)
            remote_coarrays[i] = this;//do I want to keep the global address of the local array stored?
        else
            remote_coarrays[i] = new coarray<T,NumDims>(s[i]);
    }
    delete [] s;
}
/*
template<typename T, int NumDims>
coarray<T, NumDims>::coarray(int size) {
    remote_init();
    //data = new coarray<T,NumDims-1>[size];
    for(int i = 0; i < NumDims; i++) {
        extents[i] = size;
    }
};*/


int this_image(){
    return gasnet_mynode();;
}

int num_images(){
    return gasnet_nodes();
}

