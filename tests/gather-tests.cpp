#include "coarray.h"
#include "collectives.h"

int main(int argc, char **argv){

    const size_t data_size = 16;
    local_array<double> gather_result(num_images() * data_size);
    coarray<double, 1> source(dims(data_size));
}
