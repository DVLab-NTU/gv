#ifndef ECO_MGR_CPP
#define ECO_MGR_CPP

#include "ecoMgr.h"


// top function to do ECO
void
EcoMgr::doEco(const string& oldDesignName, const string& newDesignName) {
  // read designs
  readDesigns(oldDesignName, newDesignName);
}

// read input designs
void
EcoMgr::readDesigns(const string& oldDesignName, const string& newDesignName) {
  _oldNtk = new EcoNtk;
  _newNtk = new EcoNtk;
  _oldNtk->parseNtkFile(oldDesignName);
  _newNtk->parseNtkFile(newDesignName);
}

#endif