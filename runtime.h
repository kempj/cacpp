
class cacpp_runtime {
    public:
        void wait_on_pending_writes();
        void put(int node, void *destination, void *source, size_t nbytes); 
        void get(int node, void *destination, void *source, size_t nbytes);

    private:
        vector<coarray> coarrays;//How do I make this work?
        //It doesn't need to be here if I use memory addresses.
        //actually it does. Where am I going to look up remote addresses?
        gasnet_seginfo_t *segment_info;
        int image_num = -1;
        int64_t data_size=0;//Needed globally to keep track of the beginning of each new coarray
        atomic<int> num_waiting_images {0};

};

