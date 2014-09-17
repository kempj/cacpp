#include "coarray.h"
//#include "local_array.h"
#include <chrono>

int size = 512;
int block_size = 1;
size_t num_rows;
size_t last_num_rows;

void coinit2D(coarray<int, 2> array ){
    int id = this_image();
    int tot = num_images();
    int start = id * num_rows;
    int end = (id + 1) * num_rows;
    if(id == tot - 1)
        end = size;

    for(int i = start; i < end; i++) {
        for(int j = 0; j < size; j++) {
            array[i - start][j] = i*size + j;
        }
    }
}

void comult2D( coarray<int, 2> A,
               coarray<int, 2> B,
               coarray<int, 2> C){
    int id = this_image();
    int tot = num_images();
    int section_size = num_rows;
    int row_start, row_end;
    if(id == tot - 1) {
        section_size = last_num_rows;
        row_start = size - last_num_rows;
        row_end = size;
    } else {
        section_size = num_rows;
        row_start = id * num_rows;
        row_end = (id + 1) * num_rows;
    }
    
    for(int row = 0; row < section_size; row++) {
        if(id == 0) {
            if (row % 8 == 0){
                cout << "row " << row << "( node " << id << ")" << endl;
            }
        }
        for(int col = 0; col < size; col++) {
            C[row][col] = 0;
            for(int CA = 0; CA < tot-1; CA++) {
                for(size_t inner = 0; inner < num_rows; inner++) {
                    C[row][col] = C[row][col] + A[row][inner + CA*num_rows] * B(CA)[inner][col];
                }
            }
            for(size_t inner = 0; inner < last_num_rows; inner++) {
                C[row][col] = C[row][col] + A[row][inner + (tot-1)*num_rows] * B(tot-1)[inner][col];
            }
        }
    }
    cout << "node complete: " << id << endl;
}

void comult_LA( coarray<int, 2> A,
               coarray<int, 2> B,
               coarray<int, 2> C){
    int id = this_image();
    int tot = num_images();
    int section_size = num_rows;
    int row_start, row_end;
    if(id == tot - 1) {
        section_size = last_num_rows;
        row_start = size - last_num_rows;
        row_end = size;
    } else {
        section_size = num_rows;
        row_start = id * num_rows;
        row_end = (id + 1) * num_rows;
    }

    local_array<int> tmp(last_num_rows);
    
    for(int row = 0; row < section_size; row++) {
        if(id == 0 && row % 8 == 0) {
            cout << "row " << row << "( node " << id << ")" << endl;
        }
        for(int col = 0; col < size; col++) {
            C[row][col] = 0;
            for(int CA = 0; CA < tot-1; CA++) {
                tmp = B(CA)[range()][col];
                for(size_t inner = 0; inner < num_rows; inner++) {
                    C[row][col] = C[row][col] + A[row][inner+CA*num_rows] * tmp[inner];
                }
            }
            tmp = B(tot-1)[range()][col];
            for(size_t inner = 0; inner < last_num_rows; inner++) {
                C[row][col] = C[row][col] + A[row][inner + (tot-1)*num_rows] * tmp[inner];
            }
        }
    }
    cout << "node complete: " << id << endl;
}

void comult_block( coarray<int, 2> A,
                   coarray<int, 2> B,
                   coarray<int, 2> C) {
    int id = this_image();
    int tot = num_images();
    int section_size = num_rows;
    int row_start, row_end;
    if(id == tot - 1) {
        section_size = last_num_rows;
        row_start = size - last_num_rows;
        row_end = size;
    } else {
        section_size = num_rows;
        row_start = id * num_rows;
        row_end = (id + 1) * num_rows;
    }

    local_array<int> tmp(last_num_rows);
    
    for(int row = 0; row < section_size; row++) {
        if(id == 0 && row % 8 == 0) {
            cout << "row " << row << "( node " << id << ")" << endl;
        }
        for(int col = 0; col < size; col++) {
            C[row][col] = 0;
            for(int CA = 0; CA < tot-1; CA++) {
                tmp = B(CA)[range()][col];
                for(size_t inner = 0; inner < num_rows; inner++) {
                    C[row][col] = C[row][col] + A[row][inner+CA*num_rows] * tmp[inner];
                }
            }
            tmp = B(tot-1)[range()][col];
            for(size_t inner = 0; inner < last_num_rows; inner++) {
                C[row][col] = C[row][col] + A[row][inner + (tot-1)*num_rows] * tmp[inner];
            }
        }
    }
    cout << "node complete: " << id << endl;
}

void printCA(coarray<int, 2> C){
    int team_size = num_images();
    for(size_t src = 0; src < team_size; src++) {
        if(this_image() == src) {
            cout << endl << endl << "On node " << src << endl;
            for(size_t img = 0; img < team_size - 1; img++) {
                cout << "\tImage section " << img << endl;
                for(size_t  i =0; i < num_rows; i++) {
                    cout << "\t";
                    for(size_t j=0; j < size; j++) {
                        cout << C(img)[i][j] << ", ";
                    }
                    cout << endl;
                }
            }
            cout << "\tImage section " << team_size -1 << endl;
            for(size_t  i =0; i < last_num_rows; i++) {
                    cout << "\t";
                for(size_t j=0; j < size; j++) {
                    cout << C(team_size-1)[i][j] << ", ";
                }
                cout << endl;
            }
        }
        sync_all();
    }
}

int main(int argc, char **argv) 
{
    int run_type = 0;
    if(argc >= 2)
        size = atoi(argv[1]);
    if(argc >= 3)
        run_type = atoi(argv[2]);

    cout << "run size = " << size << ", run type = " << run_type << endl;

    if(size * size > 4096){
        coarray_init(4*8*size*size, argc, argv);
    } else {
        coarray_init(3*8*1024*1024, argc, argv);
    }

    size_t id = this_image();
    size_t team_size = num_images();
    last_num_rows = size - ((team_size-1)*(size/team_size));
    num_rows = size / team_size;

    coarray<int,2> A(dims{last_num_rows,size});
    coarray<int,2> B(dims{last_num_rows,size});
    coarray<int,2> C(dims{last_num_rows,size});

    coinit2D(A);
    coinit2D(B);

    sync_all();

    std::chrono::time_point <std::chrono::system_clock> start, stop, stop2;
    start= std::chrono::system_clock::now();
    if(run_type == 0) {
        comult2D(A, B, C);
    } else {
        comult_LA(A, B, C);
    }

    stop = std::chrono::system_clock::now();

    sync_all();

    stop2 = std::chrono::system_clock::now();

    if(id == 0) {
        cout << "coarray mult time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count() << " microseconds" << endl;
        cout << "coarray mult  + barrier time: " << std::chrono::duration_cast<std::chrono::microseconds> (stop2 - start).count() 
             << " microseconds" << endl;
    }

    //cout << "\n\nC:\n";
    //printCA(C);

    coarray_exit();

    return 1;
}
