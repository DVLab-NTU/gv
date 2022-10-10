/****************************************************************************
  FileName     [ gvNtk.h ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ GV Network ]
  Author       [ ]
  Copyright    [ ]
>>>>>>> e49dc0e73d18f38a26c685cf95dad078bf400c08
****************************************************************************/

#ifndef GV_NTK_H
#define GV_NTK_H
#include "yosys.h"
#include "gvAbcMgr.h"

USING_YOSYS_NAMESPACE
 
//----------------------------------------------------------------------
// GV Ntk Defines
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// Forward Declarations
//----------------------------------------------------------------------
class GVRTLDesign;


//----------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------
extern GVRTLDesign *gvRTLDesign;


//----------------------------------------------------------------------
// Data Structure : yosys
//----------------------------------------------------------------------
class GVRTLDesign
{
   public : 
      // Constructors for GV RTL Design Instance
      GVRTLDesign() {}
      ~GVRTLDesign();

      // Ntk Reference Functions
      RTLIL::Design* getDesign() { return yosys_design; }

      // Ntk I/O Functions
      
   private : 

};


//----------------------------------------------------------------------
// Inline Functions
//----------------------------------------------------------------------

#endif

