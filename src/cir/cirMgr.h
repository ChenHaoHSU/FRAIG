/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() { clear(); } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return _vAllGates[gid]; }

   // Basic access functions
   unsigned   nPi()                 const { return _nPI; }
   unsigned   nPo()                 const { return _nPO; }
   CirPiGate* pi(const int& i)      const { assert(0 <= i && i < (int)_nPI); return (CirPiGate*)_vAllGates[i + 1];           }
   CirPoGate* po(const int& i)      const { assert(0 <= i && i < (int)_nPO); return (CirPoGate*)_vAllGates[_maxIdx + i + 1]; }
   CirPiGate* pi(const unsigned& i) const { assert(0 <= i && i < _nPI);      return (CirPiGate*)_vAllGates[i + 1];           }
   CirPoGate* po(const unsigned& i) const { assert(0 <= i && i < _nPO);      return (CirPoGate*)_vAllGates[_maxIdx + i + 1]; }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   bool parseAag(ifstream&);
   bool parsePi(ifstream&);
   bool parsePo(ifstream&);
   bool parseAig(ifstream&);
   bool parseSymbol(ifstream&);
   bool parseComment(ifstream&);
   void preProcess();
   void sortAllGateFanout();
   CirGate* queryGate(unsigned);

   // Member functions about gate lists
   void buildDfsList();
   void buildFloatingList();
   void buildUnusedList();
   void buildUndefList();
   void countAig();
   void rec_dfs(CirGate*);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   // Member functions about freeing pointers
   void clear();

private:
   // Basic Info (M I L O A)
   unsigned          _maxIdx;        // M
   unsigned          _nPI;           // I
   unsigned          _nLATCH;        // L 
   unsigned          _nPO;           // O
   unsigned          _nAIG;          // A

   // Gate Lists
   vector<CirGate*>  _vAllGates;     // List of all gates!! Can be access by idx!!
   vector<CirGate*>  _vDfsList;      // Depth-Fisrt Search List
   vector<CirGate*>  _vFloatingList; // List of all floating gates
   vector<CirGate*>  _vUnusedList;    // List of all unused gates
   vector<CirGate*>  _vUndefList;    // List of all undefined gates

   ofstream           *_simLog;
};

#endif // CIR_MGR_H
