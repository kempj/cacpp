#include <vector>
#include <array>
#include <cstddef>

using std::vector;

struct descriptor {
    descriptor( const vector<size_t>& D, const vector<size_t>& C, 
                size_t delta, size_t ts) 
              : global_offset(delta), type_size(ts), dimensions(D), codims(C) {

        for(size_t i = 0; i < dimensions.size() - 1; i++) {
            stride_multiplier.push_back(dimensions[dimensions.size()-1]);
        }
        stride_multiplier.push_back(1);
        for(int i = dimensions.size()-2; i >= 0; i--) {
            stride_multiplier[i] = dimensions[i+1] * stride_multiplier[i+1];
        }
        num_elements = dimensions[0] * stride_multiplier[0];
        total_size = num_elements * type_size;
    }

    template<size_t N>
    size_t offset_of(const std::array<size_t,N>& coords) {
        size_t val = 0;
        for(size_t i = 1; i < N; i++) {
            val += coords[i] * stride_multiplier[i];
        }
        return (val * type_size) + global_offset;
    }
    size_t num_elements;
    size_t total_size;
    size_t global_offset;
    size_t type_size;
    vector<size_t> dimensions;
    vector<size_t> stride_multiplier;
    vector<size_t> codims;
};

/*
    template<int NumDims>
    size_t size( const std::array<size_t,NumDims>& start_coord, 
                 const std::array<size_t,NumDims>& end_coord) {
        int val = 1;
        for(int i =0; i < NumDims; i++){
            val *= (end_coord[i] - start_coord[i]);
        }
    }
*/
