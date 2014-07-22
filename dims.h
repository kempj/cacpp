
struct dims {
    int size;
    int *D;

    dims(): size(0){ }
/*
template<class... Ts>
    dims(Ts...) {
        size = sizeof...(Ts);
        D = new int[size];
        int counter = 0;
        auto values = {Ts...};
        for(int val : values) {
            D[counter] = val;
            counter++;
        }
    }
    */
    dims(int dim1): size(1){
        D = new int;
        D[0] = dim1;
    }
    dims(int dim1, int dim2): size(2){
        D = new int[2];
        D[0] = dim1;
        D[1] = dim2;
    }
    dims(int dim1, int dim2, int dim3): size(3){
        D = new int[3];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
    }
    dims(int dim1, int dim2, int dim3, int dim4): size(4){
        D = new int[4];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
    }
    dims(int dim1, int dim2, int dim3, int dim4, int dim5): size(5){
        D = new int[5];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
        D[4] = dim5;
    }
    dims(int dim1, int dim2, int dim3, int dim4, int dim5, int dim6): size(6){
        D = new int[6];
        D[0] = dim1;
        D[1] = dim2;
        D[2] = dim3;
        D[3] = dim4;
        D[4] = dim5;
        D[5] = dim6;
    }
    ~dims(){
        delete[] D;
    }
};
