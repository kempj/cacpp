#include <vector>

struct codims {
    int size;
    std::vector<int> D;

    codims(): size(0){ }
    
    codims(int *extents, int extent_dims) {
        for(int i = 0; i < extent_dims; i++) {
            D.push_back(extents[i]);
        }
    }

    codims(std::initializer_list<int> l): D(l) {
        size = l.size();
    }
};
