
class remote_array {};

template<typename T>
class coarray {
    public:
        coarray(int dim);
        remote_array operator()(int);
        T operator[](int i){return local_data[i];}
    private:
        T local_data;
};

int main(int argc, char **argc) 
{
    coarray<int> test(10);
    test[0];
    return 1;
}
