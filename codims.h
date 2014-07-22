#include <vector>

struct codims {
    int size;
    //int *D;
    std::vector<int> D;

    codims(): size(0){ }

    codims(std::initializer_list<int> l): D(l) {
        size = l.size();
    }
    /*
    codims(int dim1): size(1){
        D = new int;
        D[0] = dim1;
    }
    codims(int dim1, int dim2): size(2){
        D = new int[2];
        D[0] = dim1;
        D[1] = dim2;
    }
    codims(int dim1, int dim2, int dim3): size(3){
        D = new int[3];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
    }
    codims(int dim1, int dim2, int dim3, int dim4): size(4){
        D = new int[4];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
    }
    codims(int dim1, int dim2, int dim3, int dim4, int dim5): size(5){
        D = new int[5];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
        D[4] = dim5;
    }
    codims(int dim1, int dim2, int dim3, int dim4, int dim5, int dim6): size(6){
        D = new int[6];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
        D[4] = dim5;
        D[5] = dim6;
    }
    ~codims(){
        delete[] D;
    }*/
};
