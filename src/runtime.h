#include <vector>
#include <atomic>
#include <stdexcept>
#include <iostream>

#define GASNET_PAR 1
#include "gasnet.h"
using std::vector;
using std::atomic;


struct location_data {
    size_t rt_id;
    std::vector<size_t> start_coords;
    int node_id;
};

struct descriptor {
    descriptor( const vector<size_t>& D, const vector<size_t>& C, 
                size_t delta, size_t ts) : extents(D), codims(C), offset(delta), type_size(ts) {
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
    size_t begin(const std::vector<size_t>& coords) {
        size_t val = offset;
        for(int i = 0; i < coords.size(); i++) {
            val += coords[i] * stride_multiplier[i];
        }
        return val * type_size;
    }
    size_t size(const std::vector<size_t>& coords) {
        return stride_multiplier[coords.size()-1];
    }
    size_t num_elements;
    size_t total_size;
    size_t offset;
    size_t type_size;
    vector<size_t> extents;
    vector<size_t> stride_multiplier;
    vector<size_t> codims;
};

class coarray_runtime {
    public:
        coarray_runtime( size_t segsize, int argc , char **argv );
        void wait_on_pending_writes();
        void get(void *source, void *destination, int node, size_t nbytes);
        void get(void *dest, const location_data& src);
        void put(void *source, void *destination, int node, size_t nbytes); 
        void put(void *source, const location_data& dest); 

        int get_image_id(){
            return image_num;
        }
        int get_num_images() {
            return num_images;
        }
        bool is_local(const location_data& data) {
            return data.node_id == image_num;
        }
        void* get_address(const location_data& loc) {
            void* base = segment_info[loc.node_id].addr;
            return base + handles[loc.rt_id].begin(loc.start_coords);
        }
        size_t size(const location_data& data){
            return handles[data.rt_id].size(data.start_coords);
        }
        vector<size_t>& get_codims(const location_data& data){
            return handles[data.rt_id].codims;
        }
        int coarray_setup(vector<size_t> dims, vector<size_t> codims, size_t type_size) {
            int index = handles.size();
            handles.push_back( descriptor(dims, codims, data_size, type_size));
            data_size += handles[index].total_size;
            //TODO: Do I want to round this up/align the data?
            return index;
        }
        ~coarray_runtime() {
            delete[] segment_info;
        }
        void barrier();
        void sync_images(int *image_list, int size);
        int retval = GASNET_OK;
    private:
        vector<descriptor> handles;
        gasnet_seginfo_t *segment_info;
        int image_num = -1;
        int num_images = -1;
        size_t data_size = 0;//Needed globally to keep track of the beginning of each new coarray
};


