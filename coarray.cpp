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
        coref(int size = 1) {
            for(int i = 0; i < NumDims; i++)
                extents[i] = size;
            data = new coref<T,NumDims-1>[size];
        }
        coref(void *address) {
            //TODO: this is ugly. Double check for correctness
            data = (coref<T,NumDims-1>*)address;
        }
        coref<T,NumDims-1>& operator[](int i){ 
            return data[i];
        }
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

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size):data(size){
            remote_init();
        }
        //coarray(int size[NumDims]): data(size){remote_init();}
        coarray(gasnet_seginfo_t s, int id):data(s.addr){
            address = s.addr;
            node_id = id;
        }
        coarray<T,NumDims>& operator()(int id){
            return *remote_coarrays[id];
        }
        coref<T,NumDims-1>& operator[](int i){ 
            if(gasnet_mynode() == node_id )
                return data[i];
        }
    private:
        void remote_init();
        coref<T, NumDims> data;
        coarray<T,NumDims> **remote_coarrays;
        void* address;
        int node_id;
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
            remote_coarrays[i] = new coarray<T,NumDims>(s[i], i);
        }
    }
    delete [] s;
}

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
