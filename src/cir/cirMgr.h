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
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>

#include "cirDef.h"
#include "cirGate.h"
#include "cirModel.h"
#include "cirFecGrp.h"
#include "cirSimValue.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.
extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() : _bFirstSim(false) {}
   ~CirMgr() { clear(); } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return gid < _vAllGates.size() ? _vAllGates[gid] : nullptr; }

   // Member functions about circuit construction
   bool readCircuit(const string&);

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

private:
   // Basic Info (M I L O A)
   unsigned           _maxIdx;          // M (Max gate index)
   unsigned           _nPI;             // I (Number of PIs)
   unsigned           _nLATCH;          // L (Number of LATCHes)
   unsigned           _nPO;             // O (Number of POs)
   unsigned           _nAIG;            // A (Number of AIGs)
   unsigned           _nDfsAIG;         // number of Aig in dfs list

   // Gate Lists
   vector<CirGate*>   _vPi;             // List of all PIs
   vector<CirGate*>   _vAllGates;       // List of all gates!! Can be accessed by idx!!
   vector<CirGate*>   _vDfsList;        // Depth-Fisrt Search List
   vector<CirGate*>   _vFloatingList;   // List of all floating gates
   vector<CirGate*>   _vUnusedList;     // List of all unused gates
   vector<CirGate*>   _vUndefList;      // List of all undefined gates

   // Sim log file (Do not remove it!!)
   ofstream          *_simLog;          // Log file of Simulation result

   // Simulation
   bool               _bFirstSim;       // Is the FEC group be initialized ? (i.e. ever simulated?)
   list<CirFecGrp*>   _lFecGrps;        // List of all FEC groups

   // Fraig

   ////////////////////////////////////
   //      Private Functions         //
   ////////////////////////////////////

   // Private access functions
   unsigned   nPi()                const { return _nPI; }
   unsigned   nPo()                const { return _nPO; }
   CirPiGate* pi(const int i)      const { assert(0 <= i && i < (int)_nPI); return static_cast<CirPiGate*>(_vPi[i]);                     }
   CirPoGate* po(const int i)      const { assert(0 <= i && i < (int)_nPO); return static_cast<CirPoGate*>(_vAllGates[_maxIdx + i + 1]); }
   CirPiGate* pi(const unsigned i) const { assert(i < _nPI); return static_cast<CirPiGate*>(_vPi[i]);                     }
   CirPoGate* po(const unsigned i) const { assert(i < _nPO); return static_cast<CirPoGate*>(_vAllGates[_maxIdx + i + 1]); }
   CirGate*   constGate()          const { return _vAllGates[0]; } 

   // Private functions for parsing AAG file (defined in cirParse.cpp)
   bool parse_aag(ifstream& fin);
   bool parse_pi(ifstream& fin);
   bool parse_po(ifstream& fin);
   bool parse_aig(ifstream& fin);
   bool parse_symbol(ifstream& fin);
   bool parse_comment(ifstream& fin);
   void parse_preprocess();
   CirGate* queryGate(const unsigned gid);

   // Private functions for building gate lists (defined in cirMgr.cpp)
   void buildDfsList();
   void buildFloatingList();
   void buildUnusedList();
   void buildUndefList();
   void countAig();
   void rec_dfs(CirGate* g);

   // Private common functions (defined in cirMgr.cpp)
   void delGate(CirGate* g);
   void clear();
   void sortAllGateFanout();
   void mergeGate(CirGate* aliveGate, CirGate* deadGate, bool invMerged);

   // Private functions for cirSweep and cirOptimize (defined in cirOpt.cpp)
   
   // Private functions for cirSimulation (defined in cirSim.cpp)
   bool sim_checkPattern(const string& patternStr);
   void sim_simulation(const CirModel& model);
   void sim_initClassifyFecGrp();
   void sim_classifyFecGrp();
   void sim_sweepInvalidFecGrp();
   void sim_sortFecGrps_var();
   void sim_linkGrp2Gate();
   void sim_writeSimLog(const unsigned nPatterns) const;
   void sim_print_total_fecgrp_msg() const;

   // Private functions for cirFraig (defined in cirFraig.cpp)
   void fraig_initSatSolver(SatSolver& satSolver);
   void fraig_assign_dfsOrder();
   void fraig_sort_fecgrps_dfsOrder();
   bool fraig_solve(const CirGateV& g1, const CirGateV& g2, SatSolver& satSolver);
   void fraig_collect_conuter_example(const SatSolver& satSolver, CirModel& model, const unsigned pos);
   void fraig_merge_equivalent_gates(vector<pair<CirGateV, CirGateV> >& vMergePairs);
   void fraig_refine_fecgrp();
   void fraig_proving_msg(const CirGateV& g1, const CirGateV& g2);
   void fraig_print_unsat_update_msg() const;
   void fraig_print_sat_update_msg() const;
   unsigned fraig_sat_var(const unsigned gate_var) const;

   // Util functions (defined in cirMgr.cpp)
   string bitString(size_t s) const;
};

#endif // CIR_MGR_H
