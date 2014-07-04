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
/*
void init_nodes() {
    int num_nodes = gasnet_nodes();
    if(!(*nodes)) {
        *nodes = new gasnet_seginfo_t[num_nodes];
        GASNET_SAFE(gasnet_getSegmentInfo(*nodes, num_nodes));
    }
}*/

template<typename T>
class coarray {
    public:
        //coarray(int dim, int *size);
        coarray(int size = 1);
        //~coarray();//maybe object as other nodes see it to -1 
        coarray<T>& operator()(int id){
            return remote_coarrays[id];
        }
        T& operator[](int i){
            if(local_data)
                return local_data[i];
            else {
            }
        }
    private:
        T *local_data;
        int dim;
        int *extents;
        coarray<T> **remote_coarrays;
};

static gasnet_seginfo_t *nodes;

template<typename T>
coarray<T>::coarray(int size) {
    //init_nodes();
    remote_coarrays = new coarray<T>*[gasnet_nodes()];
    dim = 1;
    local_data = new T[size];
    extents = new int[1];
    extents[0] = size;
};
/*
template<typename T>
class remote_coarray : coarray<T>{
    public:
        remote_coarray();
        T& operator[](int i){
            return tmp;
        }
        T& operator=(T arg) {
            //put remote data here
        }
        void init(gasnet_seginfo_t s){
            addr = s.addr;
        }
    private:
        void *addr;
        T tmp;
        int dim;
        int *extents;
};*/

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

    int id = this_image();
    int team_size = num_images();
    coarray<int> test(team_size);

    cout << "hello from node " << id << endl;

    if(id == 0){
        std::cout << num_images() << std::endl;
    }
    for(int i = 0; i < team_size; i++) {
        test( (id+i) % team_size)[i] = team_size * id + i; 
    }
    if(0 == id) {
        std::cout << test[0] << std::endl;
    }
    //gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    //gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    gasnet_exit(0);

    return 1;
}
