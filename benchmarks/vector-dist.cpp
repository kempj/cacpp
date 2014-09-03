#include "coarray.h"
#include <chrono>

void init(coarray<int, 1> array, int size){
    for(int i = 0; i < size; i++) {
        array[i] = i + this_image(); 
    }
}

void mult( coarray<int, 1> A, coarray<int, 1> B, int size){
    for(int i = 0; i < size; i++) {
        B[i] = A[i] * B[i];
    }
}

int main(int argc, char **argv) 
{

    coarray_init(64*1024*1024, argc, argv);
    auto id = this_image();
    auto team_size = num_images();

    size_t size = 64*1024;
    int ex[1];
    ex[0] = size;

    coarray<int,1> A(dims{size});

    coarray<int,1> B(dims{size});

    init(A, size);
    init(B, size);

    sync_all();

    std::chrono::time_point <std::chrono::system_clock> start, stop;

    if(id == 0) {
        cout << "before, A[10] = " << A[10] << endl;
    }

    start= std::chrono::system_clock::now();
    if(id == 0) {
        mult(A(0), A((id+1)%team_size), size);
        //mult(A((id+1)%team_size), A(0), size);
    }
    stop = std::chrono::system_clock::now();

    sync_all();

    if(id == 0) {
        cout << "after, A[10] = " << A((id+1)%team_size)[10] << endl;
        cout << "coarray mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;
    }

    coarray_exit();

    return 1;
}
