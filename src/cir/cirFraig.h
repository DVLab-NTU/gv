/****************************************************************************
  FileName     [ cirFraig.h ]
  PackageName  [ cir ]
  Synopsis     [ Define FRIAG related data structure ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_FRAIG_H
#define CIR_FRAIG_H

// TODO: remove this file for TODO...

class StrashKey
{
public:
   // Make sure _in0 <= _in1
   StrashKey(size_t in0, size_t in1) {
      if (in0 > in1) { _in0 = in1; _in1 = in0; }
      else { _in0 = in0; _in1 = in1; }
   }

   size_t operator() () const { return ((_in0 >> 3) + (_in1 >> 3)); }

   bool operator == (const StrashKey& k) const {
      return (_in0 == k._in0) && (_in1 == k._in1); }

private:
   size_t  _in0;
   size_t  _in1;
};

#endif // CIR_FRAIG_H
