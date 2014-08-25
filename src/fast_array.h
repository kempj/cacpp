#include <coarray.h>

template<typename T, int NumDims>
class fast_array {
    public:
        fast_array(size_t size) {
            data = new T[size];
        }
        ~fast_array() {
            delete[] data;
        }

        fast_array<T,NumDims>& operator=(T *new_data){
            if(data) {
                delete[] data;
            }
            data = new_data;
        }
        fast_array<T,NumDims>& operator=(coarray<T, NumDims> &original){
            if(!data){
                data = new T[original.size()];
            }
            //TODO: data is private to the coarray
            //TODO: coarray.size() doesn't exist.
            if(!RT->is_local(original.data)){
                RT.get(data, original.data);
            } else {
                data = std::copy(original.begin(),original.end(), data);
            }
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

