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
    // TODO: it needs an array of atomic num_waiting_images.
    // It's the end of a segment, so _all_ communication must be done.
    num_waiting_images += size;
    for(int i = 0; i < size; i++) {
        gasnet_AMRequestShort1(image_list[i], 128, -1);
    }
    GASNET_BLOCKUNTIL(num_waiting_images == 0);
}

coarray_runtime::coarray_runtime( size_t segsize, int argc, char **argv) {
    retval = gasnet_init(&argc, &argv);

    image_num = gasnet_mynode();
    num_images = gasnet_nodes();

    retval = gasnet_attach(handlers, 1, segsize, GASNET_PAGESIZE);
    segment_info = new gasnet_seginfo_t[num_images];
    retval = gasnet_getSegmentInfo(segment_info, num_images);
    if( GASNET_OK != retval) {
        throw std::runtime_error("failed to get segment info");
    }
}

void coarray_runtime::put(void *source, void *destination, int node, size_t nbytes){
    gasnet_put_bulk(node, destination, source, nbytes);
}
/*
void coarray_runtime::put(void *source, const location_data& dest) {
    size_t size = handles[dest.rt_id].size(dest.start_coords);
    size *= handles[dest.rt_id].type_size;
    put(source, get_address(dest), dest.node_id, size);
}

void coarray_runtime::get(void *dest, const location_data& src){
    size_t size = handles[src.rt_id].size(src.start_coords);
    size *= handles[src.rt_id].type_size;
    get(get_address(src), dest, src.node_id, size);
}*/

void coarray_runtime::get(void *source, void *destination, int node, size_t nbytes){
    gasnet_get_bulk(destination, node, source, nbytes);
}

void coarray_runtime::gets( void *src, void *dest, size_t dims, size_t node, size_t *count, size_t rt_id){
    size_t *src_strides = new size_t[dims-1];
    size_t *dest_strides = new size_t[dims-1];
    for(size_t i = 0; i < dims; i++) {
        src_strides[i] = handes[rt_id].type_size * handles[rt_id].stride_multiplier[ handles[rt_id].stride_multiplier.size() - 1 - i];
    }
    for(size_t i = 0; i < dims; i++) {
        dest_strides[i] = count[i];
    }
    //dest_strides[0] = handles[rt_id].num_elements;

    gasnet_gets_bulk(dest, dest_strides, node, src, src_strides, count, dims-1);
    delete[] src_strides;
    delete[] dest_strides;
}
