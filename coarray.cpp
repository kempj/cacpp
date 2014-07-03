#include <iostream>

#define GASNET_PAR 1
#include "gasnet.h"

template<typename T>
class remote_reference {
    public:
        remote_reference();
        T& operator[](int i){ return tmp;}
    private:
        int size;
        T tmp;
};

template<typename T>
class coarray {
    public:
        //codimensions can wait
        //coarray(int dim, int *extents, int codim, int *coextents);
        coarray(int dim, int *size);
        coarray(int size);
        coarray();
        //        ~coarray();

        void allocate();
        void deallocate();
        remote_reference<T> operator()(int){
            //I need to set up the remote references here.
        }

        //not sure if the const version is needed
        //const T& operator[](int i) const { return local_data[i];}
        T& operator[](int i){return local_data[i];}
    private:
        T *local_data;
        int dim;
        int *extents;
};

int this_image(){
    return gasnet_mynode();;
}

int num_images(){
    return gasnet_nodes();
}

template<typename T>
coarray<T>::coarray(int size) {
    dim = 1;
    local_data = new T[size];
    extents = new int[1];
    extents[0] = size;
};

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

int main(int argc, char **argv) 
{
    size_t segsz = GASNET_PAGESIZE;
    size_t heapsz = GASNET_PAGESIZE;

    GASNET_SAFE(gasnet_init(&argc, &argv));

    GASNET_SAFE(gasnet_attach(NULL, 0, segsz, heapsz));

    int id = this_image();
    int team_size = num_images();

    coarray<int> test(team_size);

    if(this_image() == 0){
        std::cout << num_images() << std::endl;
    }
    for(int i = 0; i < team_size; i++) {
        test( (id+i)%team_size )[i] = team_size * id + i; 
    }
    if(0 == id) {
        std::cout << test[0] << std::endl;
    }


    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    gasnet_exit(0);

    return 1;
}
