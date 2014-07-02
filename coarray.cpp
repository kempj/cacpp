#include <iostream>


template<typename T>
class remote_reference {
    public:
        remote_reference();
        T& operator[](int i);
    private:
        int size;
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
        remote_reference operator()(int){
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
    return 0;
}

int num_images(){
    return 1;
}

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

    if(this_image() == 0){
        cout << num_images() << endl;
    }
    test( (this_image() + 1) % num_images())[1] = this_image()
    return 1;
}
