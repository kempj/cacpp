#include <vector>

struct codims {
    int size;
    std::vector<size_t> D;

    codims(): size(1){
        D.push_back(1);
    }
    
    codims(int *extents, int extent_dims) {
        for(int i = 0; i < extent_dims; i++) {
            D.push_back(extents[i]);
        }
    }

    codims(std::initializer_list<size_t> l): D(l) {
        size = l.size();
    }
};
