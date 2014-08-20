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
    descriptor( vector<uint64_t> D, vector<uint64_t> C, uint64_t delta, size_t ts) : extents(D), codims(C), offset(delta), type_size(ts) {
        for(int i = 0; i < extents.size() - 1; i++) {
            stride_multiplier.push_back(extents[extents.size()-1]);
        }
        stride_multiplier.push_back(1);
        for(int i = extents.size()-2; i > 0; i--) {
            stride_multiplier[i] = extents[i+1] * stride_multiplier[i+1];
        }
        num_elements = extents[0] * stride_multiplier[0];
        total_size = num_elements * type_size;
    }
    uint64_t begin(std::vector<uint64_t> coords) {
        uint64_t val = offset;
        for(int i = 0; i < coords.size(); i++) {
            val += coords[i] * stride_multiplier[i];
        }
        return val * type_size;
    }
    uint64_t size(std::vector<uint64_t> coords) {
        return stride_multiplier[coords.size()-1];
    }
    uint64_t num_elements;
    uint64_t total_size;
    uint64_t offset;
    size_t type_size;
    vector<uint64_t> extents;
    vector<uint64_t> stride_multiplier;
    vector<uint64_t> codims;
};

struct comm_handle {
    gasnet_handle_t gasnet_handle;
    int node;
    void *destination;
    void *source;
    size_t nbytes;
    comm_handle *next;
};

class comm_handle_list {
    comm_handle *list;

    public:
        void add(comm_handle);
        void wait();
        bool has_conflict(void *destination, int node, size_t nbytes);
        comm_handle get_conflict(void *destination, int node, size_t nbytes);
        void wait(comm_handle);
};


class coarray_runtime {
    public:
        coarray_runtime( double seg_ratio = .125, int argc = 0, char **argv = NULL );
        void wait_on_pending_writes();
        void get(void *source, void *destination, int node, size_t nbytes);
        void get(void *dest, location_data src);
        void put(void *source, void *destination, int node, size_t nbytes); 
        void put(void *source, location_data dest); 

        int get_image_id(){
            return image_num;
        }
        int get_num_images() {
            return num_images;
        }
        bool is_local(location_data data) {
            return data.node_id == image_num;
        }
        void* get_address(location_data loc) {
            void* base = segment_info[loc.node_id].addr;
            return base + handles[loc.rt_id].begin(loc.start_coords);
        }
        uint64_t size(location_data data){
            return handles[data.rt_id].size(data.start_coords);
        }
        int coarray_setup(vector<uint64_t> dims, vector<uint64_t> codims, size_t type_size) {
            int index = handles.size();
            handles.push_back( descriptor(dims, codims, data_size, type_size));
            data_size += handles[index].total_size;
            return index;
        }
        ~coarray_runtime() {
            gasnet_exit(retval);
            delete[] segment_info;
        }
        void barrier();
        void sync_images(int *image_list, int size);
        void wait(coarray_data data){
        }
        void wait_all(){
            //spin on atomic
            //go through comm list until list is empty
        }
    private:
        vector<descriptor> handles;
        gasnet_seginfo_t *segment_info;
        int image_num = -1;
        int num_images = -1;
        uint64_t data_size = 0;//Needed globally to keep track of the beginning of each new coarray
        int retval = GASNET_OK;
};


