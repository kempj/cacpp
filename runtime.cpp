#include "runtime.h"

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

coarray_runtime::put(void *src, void *dest, size_t size, size_t node){

}

