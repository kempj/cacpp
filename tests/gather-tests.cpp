//#include "coarray.h"
//#include "collectives.h"
#include "cacpp.h"

size_t data_size = 16;

void print(coarray<int, 1> source) {
    if(this_image() == 0) {
        for(int img = 0; img < num_images(); img++) {
            cout << "Image(" << img << ") : ";
            for(int i = 0; i < data_size; i++) {
                cout << source(img)[i] << ", ";
            }
            cout << endl;
        }
    }
}
int main(int argc, char **argv){

    if(data_size * 8 > 4096){
        coarray_init(2*4*data_size, argc, argv);
    } else {
        coarray_init(4096,argc, argv);
    }

    local_array<int> gather_result(num_images() * data_size);
    coarray<int, 1> source(dims{data_size});//parenthesis do not work here
    coarray<int, 1> scatter_result(dims{data_size});

    for( int i = 0; i < data_size; i++ ) {
        source[i] = data_size * this_image() + i;
    }
    print(source);

    gather(gather_result, source);

    if(this_image() == 0) {
        cout << "Gather result:" << endl;
        for(int i = 0; i < num_images() * data_size; i++) {
            cout << gather_result[i] << ", ";
            gather_result[i]++;// = gather_result[i] + 1;
        }
        cout << endl;

        scatter(gather_result, scatter_result);

    }
    sync_all();
    for(int i = 0; i < num_images(); i++) {
        if(this_image() != i) {
            cout << "scatter result(" << this_image() << "): ";
            for(int i = 0; i < data_size; i++)
                cout << scatter_result[i] << ", ";
            cout << endl;
        }
    }

    sync_all();

    if(this_image() == 0) {
        collect(gather_result, scatter_result);
        cout << "collect result:" << endl;
        for(int i = 0; i < data_size; i++)
            cout << scatter_result[i] << ", ";
        cout << endl;
    }
}

