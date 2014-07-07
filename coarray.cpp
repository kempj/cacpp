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

template<typename T, int NumDims>//int NumCoDims>
class coarray {
    public:
        coarray(int size[NumDims]);
        coarray(int size = 1);
        coarray(gasnet_seginfo_t s){
            node_info = s;
        }
        coarray<T,NumDims>& operator()(int id){
            return *remote_coarrays[id];
        }
        coarray<T,NumDims-1>& operator[](int i){ 
            if ( i < extents[0])
                return data[i];
            cout << "error, accessing data outside of bounds" << endl;
        }
        //void remote_init(coarray<T, NumDims> **remote);
    private:
        coarray<T, NumDims-1> *data;
        //const static int dim = NumDims;
        int extents[NumDims];
        coarray<T,NumDims> **remote_coarrays;
        gasnet_seginfo_t node_info;
};
/*
template<typename T, int NumDims>
void 
coarray<T, NumDims>::remote_init(coarray<T, NumDims> **remote_coarrays) {
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
*/
template<typename T, int NumDims>
coarray<T, NumDims>::coarray(int size) {

    //remote_init(remote_coarrays);
    data = new coarray<T,NumDims-1>[size];
    //extents = new int[num_d];
    for(int i = 0; i < NumDims; i++) {
        extents[i] = size;
    }
};

template<typename T>
class coarray<T,1> {
    public:
        coarray(int size=1){
            local_data = new T[size];
        }
        coarray<T,1>& operator()(int id){
            return *remote_coarrays[id];
        }
        T& operator[](int i){
            return local_data[i];
        }
    private:
        T *local_data;
        coarray<T,1> **remote_coarrays;
        gasnet_seginfo_t node_info;
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
