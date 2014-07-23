#include <vector>

struct codims {
    int size;
    std::vector<int> D;

    codims(): size(0){ }

    codims(std::initializer_list<int> l): D(l) {
        size = l.size();
    }
};
