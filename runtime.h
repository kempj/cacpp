
#define GASNET_PAR 1
#include "gasnet.h"
using std::vector;

struct descriptor {
    vector<uint64_t> extents;
    vector<uint64_t> stride_multiplier;
    uint64_t size;
}

class coarray_runtime {
    public:
        coarray_runtime( double seg_ratio = .1, int argc = NULL, char **argv = NULL );
        void wait_on_pending_writes();
        void put(int node, void *destination, void *source, size_t nbytes); 
        void get(int node, void *destination, void *source, size_t nbytes);

        int get_image_id(){
            return image_num;
        }
        int get_num_images() {
            return num_images;
        }
        ~coarray_runtime() {
            gasnet_exit();
            delete[] segment_info;
        }
    private:
        vector<int> codims;
        void vector<descriptor>;
        //codimensions?
        gasnet_seginfo_t *segment_info;
        int image_num = -1;
        int num_images = -1;
        int64_t data_size = 0;//Needed globally to keep track of the beginning of each new coarray
        atomic<int> num_waiting_images {0};
        int retval = GASNET_OK;
};


void increment_num_waiting_images(gasnet_token_t token, int inc_value) {
    num_waiting_images += inc_value;
}

static gasnet_handlerentry_t handlers[] = {
    {128, (void(*)())increment_num_waiting_images}
};

