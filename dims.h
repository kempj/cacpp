#include <vector>

struct dims {
    int size;
    std::vector<int> D;

    dims(): size(0){ }

    dims(std::initializer_list<int> l): D(l) {
        size = l.size();
    }
};
