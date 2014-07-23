//This isn't needed, at least not yet:
            /*
        coref<T,1>& operator=(T* const other){
            assert(false);
            if( node_id != image_num ) {
                //send data
            }
            std::copy(other, other + total_size, data);
            return *this;
        }
        coref<T,1>& operator=(std::array<T,1> other){
            assert(other.size() == total_size);
            if( node_id != image_num ) {
                //send data
            }
            std::copy(other.begin(), other.end(), data);
            return *this;
        }*/
//-----------------------------------------------------------
        /*
        //copy:
        coref(const coref<T,NumDims>& other){
            data = other.data;
            node_id = other.node_id;
            size = other.size;
        }
        //move:
        coref(coref<T,NumDims>&& other) {
            data = other.data;
            size = other.size;
            node_id = other.node_id;
            //do I need to get rid of anything else?
        }
        //assignment:
        coref<T,NumDims>& operator=(coref<T,NumDims> other){
            data = other.data;
            //size should already be the same
            //if(other.node_id == this_image())
            //if(node_id == other.node_id)
            return *this;
        }
        */


        /*
        coref(const coref<T,0>& other){
            data = other.data;
            node_id = other.node_id;
        }
        coref(coref<T,0>&& other) {
            data = other.data;
            node_id = other.node_id;
        }
        coref<T,0>& operator=(coref<T,0> other){
            data = other.data;
            node_id = other.node_id;
            return *this;
        }*/

coref<T,0>& operator=(coref<T,0> & other){
    if(this == & other)
        return *this;
    if(other.node_id == node_id) {
        if(node_id == this_image()) { //local to local
            data = other.data;
        } else { //remote to itself
            *this = T(other);//this can be optimized;
        }
    } else {
        if(node_id != this_image() && other.node_id != this_image()){ //remote to another remote
            *this = T(other);//can be optimized
        } else if(node_id == this_image()) {  //local to remote
            *this = T(other);
        } else { //remote to local
            other = T(*this);
        }
    }
}
