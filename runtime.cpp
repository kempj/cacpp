#include "runtime.h"

atomic<int> num_waiting_images {0};

void increment_num_waiting_images(gasnet_token_t token, int inc_value) {
    num_waiting_images += inc_value;
}

static gasnet_handlerentry_t handlers[] = {
    {128, (void(*)())increment_num_waiting_images}
};

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
    if(argc == 0){
        char *tmp = "coarrayRT";
        argv = &tmp;
        argc = 1;
    }
    retval = gasnet_init(&argc, &argv);

    int seg_size = (seg_ratio * gasnet_getMaxLocalSegmentSize());
    seg_size -= seg_size % GASNET_PAGESIZE;
    retval = gasnet_attach(handlers, 1, seg_size, GASNET_PAGESIZE);

    image_num = gasnet_mynode();
    num_images = gasnet_nodes();
    segment_info = new gasnet_seginfo_t[num_images];

    retval = gasnet_getSegmentInfo(segment_info, num_images);
    if( GASNET_OK != retval) {
        throw std::runtime_error("failed to get segment info");
    }
}

void coarray_runtime::put(void *source, void *destination, int node, size_t nbytes){
    gasnet_put_bulk(node, destination, source, nbytes);
}

void coarray_runtime::put(void *source, location_data dest) {
    uint64_t size = handles[dest.rt_id].size(dest.start_coords);
    size *= handles[dest.rt_id].type_size;
    put(source, get_address(dest), dest.node_id, size);
}

void coarray_runtime::get(void *dest, location_data src){
    uint64_t size = handles[src.rt_id].size(src.start_coords);
    size *= handles[src.rt_id].type_size;
    get(get_address(src), dest, src.node_id, size);
}

void coarray_runtime::get(void *source, void *destination, int node, size_t nbytes){
    gasnet_get_bulk(destination, node, source, nbytes);
}
