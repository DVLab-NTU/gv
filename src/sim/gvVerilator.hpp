#include <string>
#include <vector>

class VerilatorAPI
{
    public:
        VerilatorAPI(string testbench, int shm_size);
        ~VerilatorAPI();
        int            get_shmid() { return _shmid; }
        vector<string> collectAssertion(string filename);
        void           genAssertionFile(vector<string> assertion_vec);
        void           reset();
        string         readData();
        void           getSequence();
        void           writeData(string data);
        void           update();
        void           evalOneCycle();
        void           printState();

    private:
        int    _shmid;
        char*  shm;
        string getDesignInfo();
        void   genInterfaceFile();
        void   verilogModeling(string testbed);
};
