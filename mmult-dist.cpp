#include "coarray.h"

void init2D(coarray<int, 2> array, int extents[2]){
    for(int i = 0; i < extents[0]; i++) {
        for(int j = 0; j < extents[1]; j++) {
            array[i][j] = i*extents[1] + j;
        }
    }
}

void coinit2D(coarray<int, 2> array, int extents[2]){
    int id = this_image();
    int tot = num_images();
    for(int i = id % tot; i < extents[0]; i += tot) {
        for(int j = 0; j < extents[1]; j++) {
            array[i][j] = i*extents[1] + j;
        }
    }
}

void comult2D( coarray<int, 2> A, int e1[2],
               coarray<int, 2> B, int e2[2],
               coarray<int, 2> C, int e3[2]){
    int height = e2[1];
    int width = e1[0];
    int id = this_image();
    int tot = num_images();

    for(int row = id%tot; row < height; row += tot) {
        for(int col = 0; col < width; col++) {
            C[row][col] = 0;
            for(int inner = 0; inner < e1[1]; inner++) {
                coref<int, 1> tmp = B(inner%tot)[inner];
                C[row][col] = C[row][col] + A[row][inner] * tmp[col];
                //C[row][col] = C[row][col] + A[row][inner] * B(inner%tot)[inner][col];
                //C[row][col] = C[row][col] + A[row][inner] * B[inner][col];
            }
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

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

    int id = this_image();
    int team_size = num_images();

    int e1[] = {3,2};
    int e2[] = {2,3};
    int e3[] = {3,3};

    coarray<int,2> A(e1);
    coarray<int,2> B(e2);
    coarray<int,2> C(e3);

    coinit2D(A, e1);
    coinit2D(B, e2);

    sync_all();
    comult2D(A, e1, B, e2, C, e3);

    sync_all();

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

    gasnet_exit(0);

    return 1;
}
