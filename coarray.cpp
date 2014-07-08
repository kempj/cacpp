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

template<typename T, int NumDims>
class coref {
    public:
        coref(int size = 1){
            for(int i = 0; i < NumDims; i++)
                extents[i] = size;
            data = new coref<T,NumDims-1>[size];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            return data[i];
        }
    private:
        coref<T,NumDims-1> *data;
        int extents[NumDims];
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
            if(this != &other)
                data = other.data;
            return *this;
        }
        coref<T,0>& operator=(T const& other){
            data = other;
            return *this;
        }
    private:
        T data;
};
/*
template<typename T>
class coref<T, 1> {
    public:
        coref(int size = 1){
            extents = size;
            data = new T[size];
        }
        T& operator[](int i){
            return data[i];
        }
        //void *address;
    private:
        T* data;
        int extents;
};
*/

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size):data(size){
            remote_init();
        }
        coarray(int size[NumDims]): data(size){
            remote_init();
        }
        coarray(gasnet_seginfo_t s){
            address = s.addr;
        }
        coarray<T,NumDims>& operator()(int id){
            return *remote_coarrays[id];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            return data[i];
        }
        int node_id;
    private:
        void remote_init();
        coref<T, NumDims> data;
        coarray<T,NumDims> **remote_coarrays;
        void* address;
};

template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init() {
    remote_coarrays = new coarray<T,NumDims>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
        if(gasnet_mynode() == i) {
            remote_coarrays[i] = this;
        } else {
            remote_coarrays[i] = new coarray<T,NumDims>(s[i]);
        }
        remote_coarrays[i]->node_id = i;
    }
    delete [] s;
}
/*
template<typename T>
class coarray<T,1> {
    public:
        coarray(int size): data(size){
            remote_init();
        }
        coarray(gasnet_seginfo_t s){
            address = s.addr;
        }
        coarray<T,1>& operator()(int id){
            return *remote_coarrays[id];
        }
        T& operator[](int i){ 
        //    if(gasnet_mynode() == node_id){
                return data[i];
        //    } else {
                //data.address = address;
        //    }
        }
        int node_id;
    private:
        void remote_init();
        coref<T,1> data;
        coarray<T,1> **remote_coarrays;
        //gasnet_seginfo_t node_info;
        void* address;
};

template<typename T>
void 
coarray<T, 1>::remote_init() {
    remote_coarrays = new coarray<T,1>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
        if(gasnet_mynode() == i) {
            remote_coarrays[i] = this;
        } else {
            remote_coarrays[i] = new coarray<T,1>(s[i]);
        }
        remote_coarrays[i]->node_id = i;
    }
    delete [] s;
}
*/

int this_image(){
    return gasnet_mynode();;
}

int num_images(){
    return gasnet_nodes();
}

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

    int out0;
    int id = this_image();
    int team_size = num_images();
    coarray<int,1> test(team_size);

    cout << "hello from node " << id << endl;

    if(id == 0){
        std::cout << num_images() << std::endl;
    }
    for(int i = 0; i < team_size; i++) {
        test( (id+i) % team_size)[i] = team_size * id + i; 
    }
    if(0 == id) {
        out0 = test[0];
        std::cout << out0 << std::endl;
    }

    gasnet_exit(0);

    return 1;
}
