#include <cassert>

#define GASNET_PAR 1
#include "gasnet.h"

extern int image_num;

template<typename T, int NumDims>
class coref {
    public:
        coref(T *address, int id, int sz[NumDims]):node_id(id), data(address) {
            std::copy(sz, sz + NumDims, &size[0]);
            total_size = size[0];
            slice_size = 1;
            for(int i = 1; i < NumDims; i++) {
                total_size *= size[i];
                slice_size *= size[i];
            }
        }
        coref<T,NumDims-1> operator[](int i){ 
            return coref<T,NumDims-1>(data + i*slice_size, node_id, &size[1]);
        }
        coref<T,NumDims> operator=(coref<T,NumDims> other){
            assert(total_size == other.total_size);
            if((node_id == image_num) && (other.node_id == image_num)) {
                std::copy(other.data, other.data + other.size, data);
                return *this;
            }
            T *tmp_data = data;
            if( other.node_id != image_num ) {//rhs is remote
                if( node_id != image_num ) {//both sides are remote
                    tmp_data = new T[size]; //TODO: this probably needs to be removed.
                }
                gasnet_get_bulk(tmp_data, other.node_id, other.data, total_size * sizeof(T));
            }
            if( node_id != image_num ) {//lhs is remote
                gasnet_put_bulk(node_id, data, tmp_data, total_size*sizeof(T));
            }
            if( tmp_data != data )
                delete[] tmp_data;
            return *this;
        }
        operator T*() {
            if(node_id == image_num) {
                return data;
            }
            //TODO
            //else {
            //    T* tmp = new T[total_size];
            //    //get remote data (needs to be blocking)
            //    return tmp;
            //}
        }
        T* get_data() const {
            if( node_id == image_num) {
                return data;
            }//else do remote get. How do I do this without more dynamic allocations?
            return NULL;
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
        coref(T *addr, int id, int sz):node_id(id), data(addr), total_size(sz) {
        }
        coref(T *addr, int id, int sz[1]):node_id(id), data(addr), total_size(sz[0]) {
        }
        const coref<T,0> operator[](const int i) const {
            return coref<T,0>(data + i, node_id);
        }
        coref<T,1> operator=(const coref<T,1> other){
            assert(total_size == other.total_size);//Throw exception
            if((node_id == image_num) && (other.node_id == image_num)) {
                std::copy(other.data, other.data + other.total_size, data);
                return *this;
            }
            T *tmp_data = data;
            if( other.node_id != image_num ) {//rhs is remote
                if( node_id != image_num ) {//both sides are remote
                    tmp_data = new T[total_size];//TODO: this probably needs to be removed. Who deallocates it?
                }
                gasnet_get_bulk(tmp_data, other.node_id, other.data, total_size * sizeof(T));
            }
            if( node_id != image_num ) {//lhs is remote
                gasnet_put_bulk(node_id, data, tmp_data, total_size*sizeof(T));
            }
            if( tmp_data != data)
                delete[] tmp_data;
            return *this;
        }
        operator T*() {
            if( node_id == image_num) {
                return data;
            }
            //TODO
            //else {
            //    T* tmp = new T[total_size];
            //    //get remote data
            //    return tmp;
            //}
        }
        T* get_data() const {
            if( node_id == image_num) {
                return data;
            }
            return NULL;
        }
    private:
        T *data;
        int node_id;
        coref();
        int total_size;
};

template<typename T>
class coref<T,0> {
    public:
        coref(T *address, int id) {
            data = address;
            node_id = id;
        }
        //TODO: define other operators, like += and >>
        const coref<T,0>& operator=(const coref<T,0> &other) {
            *data = T(other);
            return *this;
        }
        operator T() const {
            T tmp;
            if(node_id != image_num){
                gasnet_get(&tmp, node_id, data, sizeof(T));
                return tmp;
            }
            return *data;
        }
        const coref<T,0>& operator=(const T &other) const {
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
