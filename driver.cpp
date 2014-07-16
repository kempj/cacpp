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
    
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(id == 0) {
        for( int i = 0; i < team_size; i++) {
            for( int j = 0; j < team_size; j++) {
                cout << test(i)[j] << ", ";
            }
            cout << endl;
        }
    }
    
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(0 == id) {
        cout << endl << "\nTesting to see if the assignments to local corefs work:" << endl;
        test(0)[0] = 20;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(0 == id) {
        cout << "\ttest(0)[0] from image 0: " << test(0)[0] 
             << " (should be 20)" << endl;
        cout << "\ttest[0]    from image 0: " << test[0] 
             << " (should be 20)" << endl;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    if(1 == id) {
        cout << "\ttest(0)[0] from image 1: " << test(0)[0] 
             << " (should be 20)" << endl;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(0 == id) {
        cout << endl << "\nTesting to see if the assignments to remote corefs work:" << endl;
        test(1)[0] = 42;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    if(1 == id) {
        cout << "\ttest[0]    from image 1: " << test[0] 
             << " (should be 42)" << endl;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(0 == id) {
        cout << "\nTesting remote coref assignment to local coref (without parenthesis)" << endl;
        test[0] = test(1)[0];
        cout << "\ttest[0]    from image 0: " << test[0] 
             << " (should be 42)" << endl;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    if(1 == id) {
        cout << "\ttest(0)[0] from image 1: " << test(0)[0] 
             << " (should be 42)" << endl;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(0 == id) { 
        cout << "\nTesting remote coref + int assignment to local coref" << endl;
        test[1] = test(1)[0] + 1;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(0)[1] from image " << i << ": " << test(0)[1] 
                 << " (should be 43)" << endl;
        }
        gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
        gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    }

    if(0 == id) { 
        cout << "\nTesting remote coref to another remote coref" << endl;
        test(2)[0] = test(1)[0];
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(2)[0] from image " << i << ": " << test(2)[0] 
                 << " (should be 42)" << endl;
        }
        gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
        gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    }

    if(0 == id) { 
        cout << "\nTesting remote coref + int assignment to another remote coref" << endl;
        test(2)[1] = test(1)[0] + 1;
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    for(int i = 0; i < 3; i++){
        if(id == i ) {
            cout << "\ttest(2)[1] from image " << i << ": " << test(2)[1] 
                 << " (should be 43)" << endl;
        }
        gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
        gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
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
    
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    int extent2[] = {2,4};
    coarray<int,2> ca2(extent2);
    ca2[0][0] = 1;
    ca2[0][1] = 3;
    ca2[1][0] = 5;
    ca2[1][1] = 7;
    
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    if(id == 0) {
        cout << "ca2 = [[" << ca2[0][0] << ", " << ca2[0][1] << "], ["
                           << ca2[1][0] << ", " << ca2[1][1] << "]]" << endl;
    }

    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

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
        gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
        gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    }
}

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

    int id = this_image();
    int team_size = num_images();
    std::array<int,2> extents = {5, 3};
    std::array<int,2> codims = {2, 2};
    coarray<int,2> A(extents);
    coarray<int,2> B(extents);

    

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
    }
    gasnet_exit(0);
*/
    return 1;
}
