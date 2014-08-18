#include "coarray.h"
#include <chrono>

void init(coarray<int, 1> array, int size){
    for(int i = 0; i < size; i++) {
        array[i] = i;
    }
}

void add( coarray<int, 1> A, coarray<int, 1> B, int size){
    for(int i = 0; i < size; i++) {
        B[i] = A[i] + B[i];
    }
}
void mult( coarray<int, 1> A, coarray<int, 1> B, int size){
    for(int i = 0; i < size; i++) {
        B[i] = A[i] * B[i];
    }
}
void mult( int *A, int *B, int size) {
    for(int i = 0; i < size; i++) {
        B[i] =  A[i] * B[i];
    }
}
void add( int *A, int *B, int size) {
    for(int i = 0; i < size; i++) {
        B[i] =  A[i] + B[i];
    }
}

void scalar_add( int *A, int scalar, int size){
    for(int i = 0; i < size; i++) {
        A[i] += scalar;
    }
}
void scalar_mult( int *A, int scalar, int size){
    for(int i = 0; i < size; i++) {
        A[i] *= scalar;
    }
}
void scalar_add( coarray<int, 1> A, int scalar, int size){
    for(int i = 0; i < size; i++) {
        A[i] = A[i] +  scalar;
    }
}
void scalar_mult( coarray<int, 1> A, int scalar, int size){
    for(int i = 0; i < size; i++) {
        A[i] = A[i] * scalar;
    }
}

int main(int argc, char **argv) 
{

    int id = this_image();
    int team_size = num_images();

    int size = 64*1024;
    int ex[1];
    ex[0] = size;
    int scalar = 42;

    coarray<int,1> A(dims{size});

    coarray<int,1> B(dims{size});

    init(A, size);
    init(B, size);

    sync_all();

    std::chrono::time_point <std::chrono::system_clock> start, stop;

    start= std::chrono::system_clock::now();
    mult(A, B, size);
    stop = std::chrono::system_clock::now();
    cout << "coarray mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;

    init(B, size);
    start= std::chrono::system_clock::now();
    mult(A.get_data(), B.get_data(), size);
    stop = std::chrono::system_clock::now();
    cout << "normal mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count()  << " microseconds" << endl;

    init(B, size);
    start= std::chrono::system_clock::now();
    add(A, B, size);
    stop = std::chrono::system_clock::now();
    cout << "coarray add time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;

    init(B, size);
    start= std::chrono::system_clock::now();
    add(A.get_data(), B.get_data(), size);
    stop = std::chrono::system_clock::now();
    cout << "normal add time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count()  << " microseconds" << endl;


    init(A, size);
    start= std::chrono::system_clock::now();
    scalar_mult(A, scalar, size);
    stop = std::chrono::system_clock::now();
    cout << A[1] << " coarray scalar mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;

    init(A, size);
    start= std::chrono::system_clock::now();
    scalar_mult(A.get_data(), scalar, size);
    stop = std::chrono::system_clock::now();
    cout << A[1] << " normal scalar mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count()  << " microseconds" << endl;


    init(A, size);
    start= std::chrono::system_clock::now();
    scalar_add(A, scalar, size);
    stop = std::chrono::system_clock::now();
    cout << A[1] << " coarray scalar add time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;

    init(A, size);
    start= std::chrono::system_clock::now();
    scalar_add(A.get_data(), scalar, size);
    stop = std::chrono::system_clock::now();
    cout << A[1] << " normal scalar add time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count()  << " microseconds" << endl;


    sync_all();

    /*
    if(this_image() == 0) {
        cout << "\nA: " << endl;
        for(int i =0; i < e1[0]; i++) {
            for(int j=0; j < e1[1]; j++) {
                cout << A(i%team_size)[i][j] << ", ";
            }
            cout << endl;
        }
        cout << "\nB: " << endl;
        for(int i =0; i < e2[0]; i++) {
            for(int j=0; j < e2[1]; j++) {
                cout << B(i%team_size)[i][j] << ", ";
            }
            cout << endl;
        }
        cout << "\nC: " << endl;
        for(int i =0; i < e3[0]; i++) {
            for(int j=0; j < e3[1]; j++) {
                cout << C(i%team_size)[i][j] << ", ";
            }
            cout << endl;
        }
    }
    sync_all();
*/
    gasnet_exit(0);

    return 1;
}
