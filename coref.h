#include <array>
#include <cassert>

#define GASNET_PAR 1
#include "gasnet.h"

extern int image_num;

template<typename T, int NumDims>
class coref {
    public:
        coref(T *address, int id, int sz[NumDims]):node_id(id), data(address) {
            std::copy(sz, sz + NumDims, &size[0]);
            total_size = 1;
            slice_size = 1;
            for(int i = 0; i < NumDims; i++) {
                total_size *= size[i];
                if(i > 0) {
                    slice_size *= size[i];
                }
            }
        }
        coref(T *addr, int id, std::array<int,NumDims> sz): coref(addr, id, sz.data()) {}
        coref<T,NumDims-1> operator[](int i){ 
            return coref<T,NumDims-1>(data + i*slice_size, node_id, &size[1]);
        }
        coref<T,NumDims> operator=(coref<T,NumDims> other){
            //TODO: this does not work for dim > 2 for remote nodes
            assert(size == other.size);
            std::copy(other.data, other.data + other.total_size, data);
            return *this;
        }
        coref<T, NumDims-1> begin() {
            return coref<T,NumDims-1>(data, node_id, &size[1]);
        }
        coref<T, NumDims-1> end() { 
            return coref<T,NumDims-1>(data + (size[0]-1)*slice_size, node_id, &size[1]);
        }
        bool operator!=(const coref<T,NumDims> other) const {
            return !( (data == other.data) && (node_id == other.node_id) );
        }
        const coref<T,NumDims> operator++() {
            data += slice_size;
            return *this;
        }
        coref<T,1> operator*() {
            return *this;
        }
    private:
        T *data;
        int node_id;
        coref();
        int size[NumDims];
        int total_size;
        int slice_size;
};

template<typename T>
class coref <T,1> {
    public:
        coref(T *addr, int id, int sz):node_id(id), data(addr), size(sz){}
        coref(T *addr, int id, int sz[1]):node_id(id), data(addr), size(sz[0]){}

        coref(T *addr, int id, std::array<int,1> sz):node_id(id), data(addr), size(sz[0]){}

        coref<T,0> operator[](int i){ 
            return coref<T,0>(data + i, node_id);
        }
        coref<T,1> operator=(coref<T,1> other){
            assert(size == other.size);
            
            if((node_id == image_num) && (other.node_id == image_num)) {
                std::copy(other.data, other.data + other.size, data);
                return *this;
            }

            T *tmp_data = data;
            if( other.node_id != image_num ) {//rhs is remote
                if( node_id != image_num ) {//both sides are remote
                    tmp_data = new T[size];
                }
                gasnet_get_bulk(tmp_data, other.node_id, other.data, size * sizeof(T));
            }
            if( node_id != image_num ) {//lhs is remote
                //send data
                gasnet_put_bulk(node_id, data, tmp_data, size*sizeof(T));
            }
            if( tmp_data != data)
                delete[] tmp_data;
            return *this;
        }

        coref<T,1>& operator=(T* const other){
            assert(false);
            if( node_id != image_num ) {
            }
            std::copy(other, other + size, data);
            return *this;
        }
        coref<T,1>& operator=(std::array<T,1> other){
            assert(other.size() == size);
            std::copy(other.begin(), other.end(), data);
            return *this;
        }
        T* begin() {
            return &data[0];
        }
        T* end() {
            return &data[size-1];
        }
        bool operator!=(const coref<T,1> other) const {
            return !( (data == other.data) && (node_id == other.node_id) );
        }
        const coref<T,1> operator++() {
            data += size;
            return *this;
        }
        coref<T,1> operator*() {
            return *this;
        }
    private:
        T *data;
        int node_id;
        coref();
        int size;
};

template<typename T>
class coref<T,0> {
    public:
        coref(T *address, int id) {
            data = address;
            node_id = id;
        }
        //TODO: define other operators, like += and >>
        coref<T,0>& operator=(coref<T,0> & other){
            this->data = T(other);
            return *this;
        }
        operator T() {
            T tmp;
            if(node_id != image_num){
                gasnet_get(&tmp, node_id, data, sizeof(T));
                return tmp;
            }
            return *data;
        }
        coref<T,0>& operator=(T const& other){
            T tmp = other;
            if(image_num != node_id) {
                gasnet_put(node_id, data, &tmp, sizeof(T));
            } else {
                *data = other;
            }
            return *this;
        }
    private:
        coref();
        int node_id;
        T *data;
};
