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
class coarray {
    public:
        //coarray(int dim, int *size);
        coarray(int size = 1);
        coarray(gasnet_seginfo_t s){
            node_info = s;
        }
        //~coarray();//maybe object as other nodes see it to -1 
        coarray<T,NumDims>& operator()(int id){
            return *remote_coarrays[id];
        }
        T& operator[](int i){
            if(local_data)
                //Also, need to check dimensions here to return the right type.
                return local_data[i];
            else {
                //need to get remote data
                //Is it possible to detect if the object being returned is a 
                // a coarray or type T?
                // Maybe a new remote_reference type that acts as a proxy for base types?
                // What about sub-arrays, will remote_references work consistently?
                //
                // I need a special class when I only have a 1D array left; 
                // This class returns type T, while every other, higher dimension
                // returns a coarray
                
                //gasnet_get();
            }
        }
    private:
        T *local_data;
        const static int dim = NumDims;
        int *extents;
        coarray<T,NumDims> **remote_coarrays;
        gasnet_seginfo_t node_info;
};

template<typename T, int NumDims>
coarray<T, NumDims>::coarray(int size) {
    remote_coarrays = new coarray<T,NumDims>*[gasnet_nodes()];
    gasnet_seginfo_t *s =  new gasnet_seginfo_t[gasnet_nodes()];
    GASNET_SAFE(gasnet_getSegmentInfo(s, gasnet_nodes()));
    for(int i = 0; i < gasnet_nodes(); i++) {
        if(gasnet_mynode() == i)
            remote_coarrays[i] = this;
        else
            remote_coarrays[i] = new coarray<T,NumDims>(s[i]);
    }

    //dim = 1;
    local_data = new T[size];
    extents = new int[dim];
    extents[0] = size;//TODO: make this general for any number of dimensions
};

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
    coarray<int,1> test(team_size);

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
