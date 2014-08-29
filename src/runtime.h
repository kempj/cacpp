#include <vector>
#include <atomic>
#include <stdexcept>
#include <iostream>
#include <array>
#include <limits>

#include "descriptor.h"

#define GASNET_PAR 1
#include "gasnet.h"
using std::vector;
using std::atomic;


struct location_data {
    size_t rt_id;
    int node_id;
    std::vector<size_t> start_coords;
    std::vector<size_t> extents;//only used in strided communications.
};

class coarray_runtime {
        //bool is_local(const location_data &data) {
        //    return data.node_id == image_num; } 
        //size_t get_dim(const location_data &data, size_t index) {
        //    return handles[data.rt_id].dimensions[index]; }
    public:
        coarray_runtime( size_t segsize, int argc , char **argv );
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
        
        template<int NumDims>
        void* get_address(std::array<size_t,NumDims> coords, size_t rt_id, size_t node_id) {
            void* base = segment_info[node_id].addr;
            return base + handles[rt_id].offset_of(coords);
        }

        template<int NumDims>
        size_t size( std::array<size_t, NumDims> first_coord,
                     std::array<size_t, NumDims> last_coord, 
                     size_t rt_id) {
            return handles[rt_id].size(first_coord,last_coord);
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

