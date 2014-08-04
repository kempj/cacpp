#include "coarray.h"
#include <chrono>

void init2D(coarray<int, 2> array, int extents[2]){
    for(int i = 0; i < extents[0]; i++) {
        for(int j = 0; j < extents[1]; j++) {
            array[i][j] = i*extents[1] + j;
        }
    }
}

void mult2D( coarray<int, 2> A, int e1[2],
             coarray<int, 2> B, int e2[2],
             coarray<int, 2> C, int e3[2]){
    int height = e2[1];
    int width = e1[0];

    for(int row = 0; row < height; row++) {
        for(int col = 0; col < width; col++) {
            C[row][col] = 0;
            for(int inner = 0; inner < e1[1]; inner++) {
                C[row][col] = C[row][col] + A[row][inner] * B[inner][col];
            }
        }
    }
}

void mult2Drange( coarray<int, 2> A, int e1[2],
             coarray<int, 2> B, int e2[2],
             coarray<int, 2> C, int e3[2]){
    int height = e2[1];
    int width = e1[0];

    for(int row = 0; row < height; row++) {
        for(int col = 0; col < width; col++) {
            C[row][col] = 0;
            for( int inner = 0; inner < e1[1]; inner++) {
                C[row][col] = C[row][col] + A[row][inner] * B[inner][col];
            }
        }
    }
}

void mult2D( int *A, int e1[2],
             int *B, int e2[2],
             int *C, int e3[2]){
    int height = e2[1];
    int width = e1[0];

    for(int row = 0; row < height; row++) {
        for(int col = 0; col < width; col++) {
            C[row*width + col] = 0;
            for(int inner = 0; inner < e1[1]; inner++) {
                C[row*width + col] +=  A[row*width + inner] * B[inner*e2[0] + col];
            }
        }
    }
}

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

    int id = this_image();
    int team_size = num_images();

    int e1[] = {64, 64};
    int e2[] = {64, 64};
    int e3[] = {64, 64};

    coarray<int,2> A(e1);
    coarray<int,2> B(e2);
    coarray<int,2> C(e3);

    init2D(A, e1);
    init2D(B, e2);

    sync_all();
    std::chrono::time_point <std::chrono::system_clock> start, stop;
    start= std::chrono::system_clock::now();
    mult2D(A, e1, B, e2, C, e3);
    stop = std::chrono::system_clock::now();

    cout << "coarray mult time took " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() 
         << " microseconds" << endl;

    start= std::chrono::system_clock::now();
    mult2D(A.get_data(), e1, B.get_data(), e2, C.get_data(), e3);
    stop = std::chrono::system_clock::now();

    cout << "normal mult time took " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() 
         << " microseconds" << endl;

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
