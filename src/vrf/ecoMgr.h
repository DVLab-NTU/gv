#ifndef ECO_MGR_H
#define ECO_MGR_H

#include "cirMgr.h"

class EcoMgr {
public:
    EcoMgr() : _miter(0) {}
    ~EcoMgr() { delete _miter; delete _oldCir; delete _newCir;}
    void setMiter(CirMgr* miter) { _miter = miter; }
    CirMgr* getMiter() { return _miter; };
    void setOldCir(CirMgr* oldCir) { _oldCir = oldCir; }
    CirMgr* getOldCir() { return _oldCir; };
    void setNewCir(CirMgr* newCir) { _newCir = newCir; }
    CirMgr* getNewCir() { return _newCir; };
    
    void DoEco();
    void ECO_FileRead(string oldName, string newName);

private:
    CirMgr* _miter;
    CirMgr* _newCir;
    CirMgr* _oldCir;
};

#endif