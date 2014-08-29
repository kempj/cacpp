
#include <cstddef>

class range {
    public:
        bool all = false;
        size_t start = 0;
        size_t end = 0;
        range() {
            all = true;
        }
        range(size_t index) {
            end = index;
        }
        range(size_t S, size_t E) {
            start = S;
            end = E;
        }
};

