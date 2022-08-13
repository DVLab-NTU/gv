#ifndef GV_MSG_H
#define GV_MSG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class GVMsgMgr;
extern GVMsgMgr gvMsg;

class GVMsg
{
   public : 
      GVMsg(ostream& os) : _default(os) { _defaultON = true; _filename = ""; }
      ~GVMsg() {}
      bool isOutFileON() { return _outfile.is_open(); }
      void setOutFile(const string filename) { _filename = filename; }
      void stopOutFile() { if (isOutFileON()) _outfile.close(); }
      void startOutFile(bool append) {
         stopOutFile();
         if (append) _outfile.open(_filename.c_str(), ios::app);
         else _outfile.open(_filename.c_str());
      }
      void stopDefault() { _defaultON = false; }
      void startDefault() { _defaultON = true; }

      // operator overload
      template<class T> friend GVMsg& operator << (GVMsg& m, const T& s) {
         if (m._defaultON) m._default << s;
         if (m.isOutFileON()) m._outfile << s;
         if (GVMsg::_allout.is_open()) GVMsg::_allout << s;
         return m;
      }

      typedef ostream& (*StdEndLine)(ostream&);  // overload operator << for std::endl
      friend GVMsg& operator << (GVMsg& m, StdEndLine e) {
         if (m._defaultON) e(m._default);
         if (m.isOutFileON()) e(m._outfile);
         if (GVMsg::_allout.is_open()) e(GVMsg::_allout);
         return m;
      }
      static ofstream   _allout;
      static string     _allName;
   private : 
      bool              _defaultON;
      string            _filename;
      ofstream          _outfile;
      ostream&          _default;
};

enum GVMsgType
{
   GV_MSG_LOG = 0,
   GV_MSG_ERR = 1,
   GV_MSG_WAR = 2,
   GV_MSG_DBG = 3,
   GV_MSG_IFO = 4
};

class GVMsgMgr
{
   public :
      GVMsgMgr() {
         Msgs.clear();
         MsgTypeString.clear();
         Msgs.push_back(new GVMsg(cout)); MsgTypeString.push_back("");              // 0 : CmdLogMsg
         Msgs.push_back(new GVMsg(cerr)); MsgTypeString.push_back("[ERROR]: ");     // 1 : ErrorMsg
         Msgs.push_back(new GVMsg(cerr)); MsgTypeString.push_back("[WARNING]: ");   // 2 : WarningMsg
         Msgs.push_back(new GVMsg(cout)); MsgTypeString.push_back("");              // 3 : DebugMsg
         Msgs.push_back(new GVMsg(cout)); MsgTypeString.push_back("");              // 4 : InfoMsg
         Msgs[0]->stopDefault();  // NO std output needed for command Log
      }
      ~GVMsgMgr() { Msgs.clear(); }
      GVMsg& operator () (const GVMsgType type) {
         (*Msgs[type]) << MsgTypeString[type];
         return *Msgs[type];
      }
      void setOutFile(const string filename, const GVMsgType type) { Msgs[type]->setOutFile(filename); }
      void startOutFile(const GVMsgType type, bool append = false) { Msgs[type]->startOutFile(append); }
      void stopOutFile(const GVMsgType type) { Msgs[type]->stopOutFile(); }
      void startDefault(const GVMsgType type) { Msgs[type]->startDefault(); }
      void stopDefault(const GVMsgType type) { Msgs[type]->stopDefault(); }
      
      bool isAllOutFileON() { return GVMsg::_allout.is_open(); }
      void setAllOutFile(const string filename) { GVMsg::_allName = filename; }
      void startAllOutFile(bool append = false) {
         stopAllOutFile();
         if (append) GVMsg::_allout.open(GVMsg::_allName.c_str(), ios::app);
         else GVMsg::_allout.open(GVMsg::_allName.c_str());
      }
      void stopAllOutFile() { if (isAllOutFileON()) GVMsg::_allout.close(); }
      void startAllDefault() { for (unsigned i = 1; i < 5; ++i) Msgs[i]->startDefault(); }
      void stopAllDefault() { for (unsigned i = 1; i < 5; ++i) Msgs[i]->stopDefault(); }
   private : 
      vector<string> MsgTypeString;
      vector<GVMsg*> Msgs;
      pthread_mutex_t msgMutex;
};

// GV Pre-Defined Template Messages
static inline const int gvCallEmptyVirtual(const string& functionName) {
   gvMsg(GV_MSG_ERR) << "Calling virtual function " << functionName << " !!" << endl; return 0;
}

#endif

