#include <string>
#include <vector>

class VerilatorAPI
{
    public:
        VerilatorAPI(int shm_size);
        ~VerilatorAPI() {};
        int            get_shmid() { return _shmid; }
        vector<string> collectAssertion(string filename);
        void           genAssertionFile(vector<string> assertion_vec);
        void           reset(map<string, string>& map);
        string         readData();
        void           getSequence();
        void           writeData(string data);
        void           write(map<string, string>& map);
        void           update(map<string, string>& map);
        void           evalCycle(const string& num);
        void           printState();
        void           rm_shm();
        vector<string> csv2vec(string& str);
        string         vec2csv(vector<string>& vec);
        map<string, string> string2map(string& str1, string& str2);
        void           printMap(map<string, string>& map);

    private:
        int    _shmid;
        char*  shm;
        string getDesignInfo();
        void   genInterfaceFile();
        void   verilogModeling(string testbed);
};
