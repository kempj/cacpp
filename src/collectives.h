
template<typename T>
void gather(local_array<T> &result, coarray<T,1> &data){
    sync_all();
    if(this_image() == 0) {
        for(size_t i = 0; i < num_images(); i++) {
            size_t offset = data.size() * i;
            data(i).get(&result[offset]);
            //for(int j = 0; j < data.size(); j++) {
            //    result[offset + j] = data(i)[j];
            //}
        }
    }
};
/*
void scatter(local_array data, coarray result){
    //size(local_array) = size(coarray)* num_images()
};

void collect(local_array scratch, coarray data){
    //size(local_array) = size(coarray)
};
*/  
