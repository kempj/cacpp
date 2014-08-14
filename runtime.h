
#define GASNET_PAR 1
#include "gasnet.h"
using std::vector;

struct descriptor {
    descriptor( vector<int> D, vector<int> C, uint64_t delta) :
                   extents(D),     codims(C),   offset(delta) {
        for(int i = 0; i < extents.size() - 1; i++) {
            stride_multiplier.push_back(extents[extents.size()-1]);
        }
        stride_multiplier.push_back(1);
        for(int i = extents.size()-2; i > 0; i--) {
            stride_multiplier[i] = extents[i+1] * stride_multiplier[i+1];
        }
        size = extents[0] * stride_multiplier[0];
    }
    vector<int> extents;
    vector<uint64_t> stride_multiplier;
    uint64_t size;
    uint64_t offset;
    vector<int> codims;
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
        int coarray_setup(vector<uint64_t> dims, vector<uint64_t> codims) {
            data_size += descriptor.size;
            coarray_descriptors.push_back(new_descriptor);
        }
        ~coarray_runtime() {
            gasnet_exit();
            delete[] segment_info;
        }
    private:
        void vector<descriptor> coarray_descriptors;
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

