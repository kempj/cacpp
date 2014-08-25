#include <coarray.h>

template<typename T, int NumDims>
class local_array {
    public:
        local_array(size_t size) {
            data = new T[size];
        }
        ~local_array() {
            delete[] data;
        }

        local_array<T,NumDims>& operator=(T *new_data){
            if(data) {
                delete[] data;
            }
            data = new_data;
        }
        local_array<T,NumDims>& operator=(coarray<T, NumDims> &original){
            if(!data){
                data = new T[original.size()];
            }
            original.copy_to(data);
        }
        T operator[](size_t idx) {
            return data[idx];
        }
    private:
        //Do I benefit from forcing the user to specify the size
        // by making the default constructor private?

        //TODO: use smart pointer?
        T* data;
};

