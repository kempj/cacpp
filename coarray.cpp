#include <iostream>

class remote_reference {};

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
        remote_reference operator()(int);
        //not sure if the const version is needed
        //const T& operator[](int i) const { return local_data[i];}
        T& operator[](int i){return local_data[i];}
    private:
        T *local_data;
        int dim;
        int *extents;
};

template<typename T>
coarray<T>::coarray(int size) {
    dim = 1;
    local_data = new T[size];
    extents = new int[1];
    extents[0] = size;

};

int main(int argc, char **argv) 
{
    coarray<int> test(10);
    test[0] = 42;
    std::cout << test[0] << std::endl;
    return 1;
}
