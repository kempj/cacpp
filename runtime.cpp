#include "runtime.h"

void coarray_runtime::barrier() {
    gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
    int status = gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
    //if(GASNET_OK != status)
        //cout << "error while syncing all" << endl;
}

void coarray_runtime::sync_images(int *image_list, int size) {
    num_waiting_images += size;
    for(int i = 0; i < size; i++) {
        gasnet_AMRequestShort1(image_list[i], 128, -1);
    }
    GASNET_BLOCKUNTIL(num_waiting_images == 0);
}

coarray_runtime::coarray_runtime(double seg_ratio, int argc, char **argv) {
    retval = gasnet_init(&argc, &argv);

    int seg_size = (seg_ratio * gasnet_getMaxLocalSegmentSize());
    retval = gasnet_attach(handlers, 1, seg_size, GASNET_PAGESIZE);

    image_num = gasnet_mynode();
    num_images = gasnet_nodes();
    segment_info = new gasnet_seginfo_t[num_images];

    retval = gasnet_getSegmentInfo(segment_info, num_images);
    if( GASNET_OK != retval) {
        throw std::runtime_error("failed to get segment info");
    }
}

void coarray_runtime::put(int node, void *destination, void *source, size_t nbytes){

}

