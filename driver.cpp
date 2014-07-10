#include "coarray.h"

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

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
    
    if(0 == id) {
        cout << endl << "Testing to see if the assignments to remote corefs work:" << endl;
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
        cout << "Testing remote coref assignment to local coref (without parenthesis)" << endl;
        test[0] = test(1)[0];
        cout << test(1)[0] << endl;
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
        cout << "Testing remote coref + int assignment to local coref" << endl;
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
        cout << "Testing remote coref to another remote coref" << endl;
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
        cout << "Testing remote coref + int assignment to another remote coref" << endl;
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
        cout << "Testing remote coref assignment to local int" << endl;
        int tmp = test(1)[0];
        cout << "tmp should = 42: " << tmp << endl;
    }
    
    gasnet_exit(0);

    return 1;
}
