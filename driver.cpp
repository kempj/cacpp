#include "coarray.h"

void test1() {
    int id = this_image();
    int team_size = num_images();
    int extents[1];
    extents[0] = team_size;
    coarray<int,1> test(extents);

    for(int i = 0; i < team_size; i++) {
        test( (id+i) % team_size)[id] =  id ; 
    }
    
    sync_all();

    if(id == 0) {
        for( int i = 0; i < team_size; i++) {
            for( int j = 0; j < team_size; j++) {
                cout << test(i)[j] << ", ";
            }
            cout << endl;
        }
    }
    
    sync_all();

    if(0 == id) {
        cout << endl << "\nTesting to see if the assignments to local corefs work:" << endl;
        test(0)[0] = 20;
    }
    sync_all();

    if(0 == id) {
        cout << "\ttest(0)[0] from image 0: " << test(0)[0] 
             << " (should be 20)" << endl;
        cout << "\ttest[0]    from image 0: " << test[0] 
             << " (should be 20)" << endl;
    }
    sync_all();
    if(1 == id) {
        cout << "\ttest(0)[0] from image 1: " << test(0)[0] 
             << " (should be 20)" << endl;
    }
    sync_all();

    if(0 == id) {
        cout << endl << "\nTesting to see if the assignments to remote corefs work:" << endl;
        test(1)[0] = 42;
    }
    sync_all();
    if(1 == id) {
        cout << "\ttest[0]    from image 1: " << test[0] 
             << " (should be 42)" << endl;
    }
    sync_all();

    if(0 == id) {
        cout << "\nTesting remote coref assignment to local coref (without parenthesis)" << endl;
        test[0] = test(1)[0];
        cout << "\ttest[0]    from image 0: " << test[0] 
             << " (should be 42)" << endl;
    }
    sync_all();
    if(1 == id) {
        cout << "\ttest(0)[0] from image 1: " << test(0)[0] 
             << " (should be 42)" << endl;
    }
    sync_all();

    if(0 == id) { 
        cout << "\nTesting remote coref + int assignment to local coref" << endl;
        test[1] = test(1)[0] + 1;
    }
    sync_all();
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(0)[1] from image " << i << ": " << test(0)[1] 
                 << " (should be 43)" << endl;
        }
        sync_all();
    }

    if(0 == id) { 
        cout << "\nTesting remote coref to another remote coref" << endl;
        test(2)[0] = test(1)[0];
    }
    sync_all();
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(2)[0] from image " << i << ": " << test(2)[0] 
                 << " (should be 42)" << endl;
        }
        sync_all();
    }

    if(0 == id) { 
        cout << "\nTesting remote coref + int assignment to another remote coref" << endl;
        test(2)[1] = test(1)[0] + 1;
    }
    sync_all();
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(2)[1] from image " << i << ": " << test(2)[1] 
                 << " (should be 43)" << endl;
        }
        sync_all();
    }

    if(0 == id) {
        cout << "\nTesting remote coref assignment to local int" << endl;
        int tmp = test(1)[0];
        cout << "tmp should = 42: " << tmp << endl;
    }
    
}

void test2() {
    int id = this_image();
    int team_size = num_images();
    int extents[2] = {3, 5};
    coarray<int,2> test2D(extents);
    
    //int counter = 0;
    for(int i = 0; i < extents[0]; i++) {
        for(int j = 0; j < extents[1]; j++) {
            //test2D[i][j] = counter;
            //counter++;
            if(id < extents[0]){
                test2D( (id+i) % team_size)[id][j] = j + id;
            }
        }
    }

    /*
     if(id == 0) {
        for(int i = 0; i < extents[0]; i++) {
            for(int j = 0; j < extents[1]; j++) {
                cout << test2D[i][j] << ", ";
            }
            cout << endl;
        }
        for(int i = 0; i < extents[1] * extents[0]; i++) {
            cout << test2D[0][i] << ", ";
        }
        cout << endl;
    }
    */
    
    sync_all();

    int extent2[] = {2,4};
    coarray<int,2> ca2(extent2);
    ca2[0][0] = 1;
    ca2[0][1] = 3;
    ca2[1][0] = 5;
    ca2[1][1] = 7;
    
    sync_all();

    if(id == 0) {
        cout << "ca2 = [[" << ca2[0][0] << ", " << ca2[0][1] << "], ["
                           << ca2[1][0] << ", " << ca2[1][1] << "]]" << endl;
    }

    sync_all();

    for( int i = 0; i < team_size; i++) {
        if(i == id) {
            for( int j = 0; j < extents[0]; j++) {
                for( int k = 0; k < extents[1]; k++) {
                    cout << test2D[j][k] << ", ";
                }
                cout << endl;
            }
            cout << endl;
        }
        sync_all();
    }
}

void test3() {

    int id = this_image();
    int team_size = num_images();
    int extents[] = {4,4};
    coarray<int,2> A(dims{4,4}, codims{2,2});
    coarray<int,2> B(dims{4,4}, codims{2,2});
    for(int i = 0; i < extents[1]; i++) {
        A[0][i] = i;
        B[0][i] = i + extents[0];
        A[1][i] = -1;
        B[1][i] = -2;
    }

    cout << "A[0] before: ";
    for(int i = 0; i < extents[1]; i++) {
        cout << A[0][i] << ", ";
    }
    cout << endl;
    cout << "B[0] before: ";
    for(int i = 0; i < extents[1]; i++) {
        cout << B[0][i] << ", ";
    }
    cout << endl;

    A[1] = B[0];
    B[1] = A[0];

    cout << "A[0] after : ";
    for(int i = 0; i < extents[1]; i++) {
        cout << A[0][i] << ", ";
    }
    cout << endl;
    cout << "A[1] after : ";
    for(int i = 0; i < extents[1]; i++) {
        cout << A[1][i] << ", ";
    }
    cout << endl;
    cout << "B[0] after : ";
    for(int i = 0; i < extents[1]; i++) {
        cout << B[0][i] << ", ";
    }
    cout << endl;
    cout << "B[1] after : ";
    for(int i = 0; i < extents[1]; i++) {
        cout << B[1][i] << ", ";
    }
    cout << endl;

/*
    cout << "A: " << endl;
    for(int i =0; i < 5; i++) {
        A[i].print();
    }
    cout << "B: " << endl;
    for(int i =0; i < 3; i++) {
        B[i].print();
    }
    cout << "C: " << endl;
    for(int i =0; i < 5; i++) {
        C[i].print();
    }*/
}
void test4() {
    coarray<int,2> A(dims{4,4}, codims{1,2});
    coarray<int,2> B(dims{4,4}, codims{1,2});
    
    if(this_image() == 0) {
        A(0,0)[0][0] = 1;
        A(0,1)[0][0] = 2;
        A(1,0)[0][0] = 3;
        A(1,1)[0][0] = 4;
    }
    sync_all();

    for(int i = 0; i < 4; i++) {
        if(this_image() == i) {
            cout << "A[0][0] = " <<  A[0][0] << endl;
        }
        sync_all();
    }

    if(this_image() == 0) {
        cout << "getting A(0,1)[0]" << endl;
        A[1] = A(0,1)[0];
        cout << "getting A(1,0)[0]" << endl;
        A[2] = A(1,0)[0];
        cout << "getting A(1,1)[0]" << endl;
        A[3] = A(1,1)[0];
    }
    sync_all();

    if(this_image() == 0) {
        for(int i = 0; i < 4; i++) {
            cout << "A[" << i << "] :" << endl;
            for(int j = 0; j < 3; j++) {
                cout << A[i][j] << ", ";
            }
            cout << A[i][3] << endl;
        }

        //for(int i = 0; i < 4; i++) {
        for(auto row : A) {
            for(auto entry : row) {
                cout << entry << ", ";
            }
            cout << row[3] << endl;
        }
    }

}

int main(int argc, char **argv) 
{

    int id = this_image();
    int team_size = num_images();

    init_runtime(argc,argv);

    sync_all();

    coarray<int,2> A(dims{4,4}, codims{1,2});

    cout << "GASNET_PAGESIZE = " << GASNET_PAGESIZE << endl;
    cout << "Max local segment size: " << gasnet_getMaxLocalSegmentSize() << endl;
    cout << "current segment size = " << size_local_shared_memory() << endl;
    int *b;
    b = A(0);

    A(0)[0][0] = 42;

    for(int i =0; i < num_images(); i++) {
        sync_all();
        if(this_image() == i) {
            cout << "From image " << i << ", A(0)[0][0] = " << A(0)[0][0] << endl;
        }
    }

   /* 
    if(this_image() == 0)
        cout << endl << "test1" << endl;
    test1();

    sync_all();
    
    if(this_image() == 0)
        cout << endl << "test2" << endl;
    test2();

    sync_all();

    if(this_image() == 0)
        cout << endl << "test3" << endl;
    test3();
    */

    sync_all();

    gasnet_exit(0);

    return 1;
}
