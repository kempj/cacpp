#include <vector>
#include <cassert>

struct dims {
    int size;
    std::vector<size_t> D;

    dims(): size(0){ }
    
    dims(int *extents, int extent_dims) {
        for(int i = 0; i < extent_dims; i++) {
            D.push_back(extents[i]);
        }
    }

    dims(std::initializer_list<size_t> l): D(l) {
        size = l.size();
    }
};
