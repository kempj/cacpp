
template<typename T>
void gather(local_array<T> &result, coarray<T,1> &data){
    sync_all();
    if(this_image() == 0) {
        for(size_t i = 0; i < num_images(); i++) {
            data(i).get(&result[data.size() * i]);
        }
    }
};

template<typename T>
void scatter(local_array<T> &data, coarray<T,1> &result){
    if(this_image() == 0) {
        for(size_t i = 0; i < num_images(); i++) {
            result(i).put(&data[result.size() * i]);
            //result(i) = data[result.size() * i];
        }
    }
};

template<typename T>
void collect(local_array<T> &scratch, coarray<T,1> &data){
    for(int img = 1; img < num_images(); img++) {
        scratch = data(img);
        for(int i = 0; i < data.size(); i++) {
           //data[i] += scratch[i];
           data[i] = data[i] + scratch[i];
        }
    }
};
