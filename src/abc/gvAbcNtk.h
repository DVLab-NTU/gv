#ifndef GV_ABC_NTK_H
#define GV_ABC_NTK_H

using namespace std;

class AbcMgr;
class AbcNtk;

typedef struct Abc_Ntk_t_ Abc_Ntk_t;


class AbcNtk
{
    public:
        AbcNtk() {}
        ~AbcNtk() {}
    
    private:
        Abc_Ntk_t*          pNtk;
        static AbcMgr*      _AbcMgr;
};


#endif