/****************************************************************************
  FileName     [ gvNtk.h ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ GV Network ]
  Author       [ ]
  Copyright    [ ]
****************************************************************************/

#ifndef GV_NTK_H
#define GV_NTK_H

#include "kernel/yosys.h"

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
GVRTLDesign *gvRTLDesign;


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

