#include <vector>
#include <atomic>
#include <stdexcept>

#define GASNET_PAR 1
#include "gasnet.h"
using std::vector;
using std::atomic;

struct location_data {
    uint64_t rt_id;
    std::vector<uint64_t> start_coords;
    int node_id;
};

struct descriptor {
    descriptor( vector<uint64_t> D, vector<uint64_t> C, uint64_t delta) : extents(D), codims(C), offset(delta) {
        for(int i = 0; i < extents.size() - 1; i++) {
            stride_multiplier.push_back(extents[extents.size()-1]);
        }
        stride_multiplier.push_back(1);
        for(int i = extents.size()-2; i > 0; i--) {
            stride_multiplier[i] = extents[i+1] * stride_multiplier[i+1];
        }
        size = extents[0] * stride_multiplier[0];
    }
    vector<uint64_t> extents;
    vector<uint64_t> stride_multiplier;
    uint64_t size;
    uint64_t offset;
    vector<uint64_t> codims;
};

class coarray_runtime {
    public:
        coarray_runtime( double seg_ratio = .125, int argc = 0, char **argv = NULL );
        void wait_on_pending_writes();
        void put(int node, void *destination, void *source, size_t nbytes); 
        void get(int node, void *destination, void *source, size_t nbytes);

        int get_image_id(){
            return image_num;
        }
        int get_num_images() {
            return num_images;
        }
        bool is_local_node(int node_id) {
            return node_id == image_num;
        }
        int coarray_setup(vector<uint64_t> dims, vector<uint64_t> codims) {
            int index = coarray_descriptors.size();
            coarray_descriptors.push_back( descriptor(dims, codims, data_size));
            data_size += coarray_descriptors[index].size;
            return index;
        }
        ~coarray_runtime() {
            gasnet_exit(retval);
            delete[] segment_info;
        }
        void barrier();
        void sync_images(int *image_list, int size);
    private:
        vector<descriptor> coarray_descriptors;
        gasnet_seginfo_t *segment_info;
        int image_num = -1;
        int num_images = -1;
        int64_t data_size = 0;//Needed globally to keep track of the beginning of each new coarray
        int retval = GASNET_OK;
};


