
class cacpp_runtime {
    public:
        void wait_on_pending_writes();
        void write(int node, void *destination, void *source, size_t nbytes); 
        void read(int node, void *destination, void *source, size_t nbytes);

    private:
        vector<coarray> coarrays;//How do I make this work?
};

