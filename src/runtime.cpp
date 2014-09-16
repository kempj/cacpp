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

void coarray_runtime::get(void *source, void *destination, int node, size_t nbytes){
    gasnet_get_bulk(destination, node, source, nbytes);
}

void coarray_runtime::gets( void *src, void *dest, size_t dims, size_t node, size_t *count, size_t rt_id){
    size_t stride_size = dims -1;
    size_t *src_strides = new size_t[stride_size];
    size_t *dest_strides = new size_t[stride_size];
    size_t data_size = handles[rt_id].type_size;
    //size_t dest_stride = count[0] * data_size;

    for(size_t i = 0; i < stride_size; i++) {
        //src_strides[i] = handles[rt_id].stride_multiplier[ handles[rt_id].stride_multiplier.size() - 2 - i];
        src_strides[i] = data_size * handles[rt_id].stride_multiplier[ handles[rt_id].stride_multiplier.size() - 2 - i];
        dest_strides[i] = data_size;
    }
    dest_strides[0] = count[0];
   /* 
    barrier();
    if(image_num == 0) {
        cout << "========== image 0 ==========" << endl;
        for(size_t i = 0; i < stride_size; i++) {
            cout << "src_strides[" << i << "] = " << src_strides[i] << endl;
        }
        cout << "node = " << node << endl;
        cout << "src = " << src << endl;
        cout << "dest  = " << dest << endl;
        cout << "count = " << count << endl;
        cout << "stride_levels = " << dims-1 << endl;
        for(size_t i = 0; i < dims; i++) {
            cout << "count[" << i << "] = " << count[i] << endl;
        }
        cout << "==========" << endl;
    }
    barrier();
    if(image_num != 0) {*/
        cout << "========== image 1 ==========" << endl;
        for(size_t i = 0; i < stride_size; i++) {
            cout << "src_strides[" << i << "] = " << src_strides[i] << endl;
            cout << "dest_strides[" << i << "] = " << dest_strides[i] << endl;
        }
        //cout << "dest stride = " << dest_stride << endl;
        cout << "node = " << node << endl;
        cout << "src = " << src << endl;
        cout << "dest  = " << dest << endl;
        cout << "count = " << count << endl;
        cout << "stride_levels = " << dims - 1 << endl;
        for(size_t i = 0; i < dims; i++) {
            cout << "count[" << i << "] = " << count[i] << endl;
        }
        cout << "==========" << endl;
    //}
    //barrier();
    

    gasnet_gets_bulk(dest, dest_strides, node, src, src_strides, count, dims-1);
    //gasnet_gets_bulk(dest, &dest_stride, node, src, src_strides, count, dims-1);
    delete[] src_strides;
}
