#include "coarray.h"

int main(int argc, char **argv) 
{
    GASNET_SAFE(gasnet_init(&argc, &argv));
    GASNET_SAFE(gasnet_attach(NULL, 0, GASNET_PAGESIZE, GASNET_PAGESIZE));

    int id = this_image();
    int team_size = num_images();
    coarray<int,1> test(team_size);

    //cout << "hello from node " << id << endl;

    //if(id == 0){
    //    std::cout << "number of images: " <<  num_images() << std::endl;
    //}

    test(0)[id] = 42 + id;
    
    if(0 == id) {
        cout << "before barrier" << endl;
        for(int i = 0; i < team_size; i++) {
            std::cout << "data at" << &test[i].data << ": " << test[i] << std::endl;
        }
    }
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);

    //for(int i = 0; i < team_size; i++) {
    //    test( (id+i) % team_size)[i] = team_size * id + i; 
    //}
    if(0 == id) {
       cout << "after barrier" << endl;
        for(int i = 0; i < team_size; i++) {
            std::cout << "data at" << &test[i].data << ": " << test[i] << std::endl;
        }
    }

    gasnet_exit(0);

    return 1;
}
