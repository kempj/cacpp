#include <vector>
#include <atomic>
#include <stdexcept>
#include <iostream>
#include <array>
#include <limits>

#include "descriptor.h"

#define GASNET_PAR 1
#include "gasnet.h"
#include "gasnet_vis.h"

#include <iostream>
using std::cout;
using std::endl;

using std::vector;
using std::atomic;


class coarray_runtime {
    public:
        coarray_runtime( size_t segsize, int argc , char **argv );
        void get(void *source, void *destination, int node, size_t nbytes);
        void put(void *source, void *destination, int node, size_t nbytes); 
        void gets( void *source, void *destination, size_t stride_levels, size_t node, size_t *count, size_t rt_id);
        //void gets( void *source, size_t src_strides[], 
        //           void *destination, size_t dest_strides, 
        //           size_t stride_levels, int node, size_t nbytes);
        void puts( void *source, size_t src_strides[], 
                   void *destination, size_t dest_strides, 
                   size_t stride_levels, int node, size_t nbytes);

        size_t get_image_id(){
            return image_num;
        }
        size_t get_num_images() {
            return num_images;
        }
        template<size_t N>
        void* get_address(std::array<size_t,N> coords, size_t rt_id, size_t node_id) {
            void* base = segment_info[node_id].addr;
            //cout << "get_address rt_id = " << rt_id << ", base = " << base
            //     << ", offset = " << handles[rt_id].offset_of(coords) << endl;
            return base + handles[rt_id].offset_of(coords);
        }
        void* get_address( size_t rt_id, size_t node_id) {
            return  segment_info[node_id].addr + handles[rt_id].global_offset;
        }
        size_t size(size_t rt_id) {
            return handles[rt_id].num_elements;
        }
        size_t get_dim(size_t rt_id, size_t index) {
            return handles[rt_id].dimensions[index];
        }
        vector<size_t>& get_codims(size_t rt_id){
            return handles[rt_id].codims;
        }
        int coarray_setup(vector<size_t> dims, vector<size_t> codims, size_t type_size) {
            int index = handles.size();
            handles.push_back( descriptor(dims, codims, data_size, type_size));
            data_size += handles[index].total_size;
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

