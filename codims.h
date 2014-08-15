#include <vector>

//TODO: add a shape object
struct codims {
    int size;
    std::vector<uint64_t> D;

    codims(): size(0){ }
    
    codims(int *extents, int extent_dims) {
        for(int i = 0; i < extent_dims; i++) {
            D.push_back(extents[i]);
        }
    }

    codims(std::initializer_list<uint64_t> l): D(l) {
        size = l.size();
    }
};
