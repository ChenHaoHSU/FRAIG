/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cmath>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHashMap.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   // Tuned parameter 'max_fail':
   //   If #FECgroups remains the same continually 'max_fail' times,
   //   then stop random simulation.
   const unsigned max_fail  = 3 + 3 * log10((double)_vDfsList.size());

   unsigned nPatterns = 0; // accumulate number of sim patterns
   unsigned nFail = 0; // fail to make #fecGrps change
   unsigned nOldGrps = 0; // previous #FEC group

   CirModel model(_nPI);

   while (nFail < max_fail) {
      // Randomly generate patterns
      model.random();

      // Simulate the circuit
      sim_simulation(model);

      // Write to the output file _simLog
      sim_writeSimLog(SIM_CYCLE);

      sim_printMsg_totalFecGrp();
      nPatterns += SIM_CYCLE;

      // Update termination info
      nFail = (nOldGrps == _lFecGrps.size() ? (nFail + 1) : 0);
      nOldGrps = _lFecGrps.size();
   }

   sim_sortFecGrps_var();
   sim_linkGrp2Gate();

   cout << flush << "\r" << nPatterns << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   unsigned periodCnt = 0; // SIM_CYCLE, a period
   unsigned nPatterns = 0;
   string patternStr;

   CirModel model(_nPI);

   while (true) {

      if ( !(patternFile >> patternStr) ) {
         // simulate here one more time, then break
         if (periodCnt != 0) {
            // Simulate the circuit
            sim_simulation(model);

            // Write to the output file _simLog
            sim_writeSimLog(periodCnt);
            sim_printMsg_totalFecGrp();
            nPatterns += periodCnt;
         }
         break;
      }

      // Check if pattern is valid
      if (!sim_checkPattern(patternStr)) break;

      // Set pattern value to model
      model.setPattern(patternStr, periodCnt++);

      // Simulate immediately, if 64 patterns are collected.
      if (periodCnt == SIM_CYCLE) {
         sim_simulation(model);
         sim_writeSimLog(periodCnt);
         sim_printMsg_totalFecGrp();
         nPatterns += periodCnt;
         periodCnt = 0;
         model.reset();
      }
   }

   sim_sortFecGrps_var();
   sim_linkGrp2Gate();
   
   cout << flush << "\r";
   fprintf(stdout, "%u patterns simulated.\n", nPatterns);
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
bool
CirMgr::sim_checkPattern(const string& patternStr)
{
   // Error Handling:
   //    1. Length of pattern string == nPI
   //    2. Pattern string consists of '0' or '1'.
   // 
   // 1. Length check
   if (patternStr.length() != _nPI) {
      fprintf(stderr, "\nError: Pattern(%s) length(%lu) does not match the number of inputs(%u) in a circuit!!\n", 
         patternStr.c_str(), patternStr.length(), _nPI);
      return false;
   }
   // 2. Char check
   for (unsigned i = 0, n = patternStr.length(); i < n; ++i) {
      if (patternStr[i] != '0' && patternStr[i] != '1') {
         fprintf(stderr, "\nError: Pattern(%s) contains a non-0/1 character(\'%c\').\n", 
            patternStr.c_str(), patternStr[i]);
         return false;
      }
   } 
   return true;
}

void 
CirMgr::sim_simulation(const CirModel& model) 
{
   // Set simulation patterns to PIs
   for (unsigned i = 0, n = model.size(); i < n; ++i)
      pi(i)->setValue(model[i]);

   // Calculate sim value of every gate in DFS list
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i)
      _vDfsList[i]->calValue();

   // Classify gates into FEC groups
   if (!_bFirstSim) {
      sim_firstClassifyFecGrp();
      _bFirstSim = true;
   } else {
      sim_classifyFecGrp();
   }
}

void 
CirMgr::sim_firstClassifyFecGrp()
{
   CirGate* g = nullptr;
   CirFecGrp* queryGrp = nullptr;
   HashMap<CirInitSimValue, CirFecGrp*> hash;
   list<CirFecGrp*> lCandGrp;
   hash.init(getHashSize(_vDfsList.size()));

   // Const gate (must be inside whether it is in dfsList or not)
   queryGrp = new CirFecGrp;
   lCandGrp.push_back(queryGrp);
   queryGrp->candidates().emplace_back(constGate());
   hash.forceInsert(CirInitSimValue(constGate()->value()), queryGrp);
   
   // Aig gates
   for (unsigned i = 0, n = _vDfsList.size(); i < n && (g = _vDfsList[i]); ++i) {
      // Skip non-AIG gates
      if (!g->isAig()) continue;
      if (hash.check(CirInitSimValue(g->value()), queryGrp)) {
         queryGrp->candidates().emplace_back(g, g->value() != queryGrp->repValue());
      } else {
         queryGrp = new CirFecGrp;
         lCandGrp.push_back(queryGrp);
         queryGrp->candidates().emplace_back(g);
         hash.forceInsert(CirInitSimValue(g->value()), queryGrp);
      }
   }

   // Collect valid FEc group from hash
   for (auto iter2 = lCandGrp.begin(); iter2 != lCandGrp.end(); ++iter2)
      if ((*iter2)->isValid())
         _lFecGrps.push_front(*iter2);
      else
         delete *iter2;
}

void 
CirMgr::sim_classifyFecGrp()
{
   unsigned i, n;
   size_t value, oriValue;
   CirGate* g;
   CirFecGrp *queryGrp, *oriGrp;
   list<CirFecGrp*> lCandGrp;
   HashMap<CirSimValue, CirFecGrp*> hash;

   for (auto iter = _lFecGrps.begin(); iter != _lFecGrps.end(); iter = _lFecGrps.erase(iter)) {

      lCandGrp.clear();

      oriGrp = *iter;
      hash.init(getHashSize(oriGrp->size()));

      for (i = 0, n = oriGrp->size(); i < n && (g = oriGrp->candGate(i)); ++i) {

         if (g == nullptr) continue;

         oriValue = g->value();
         value = oriGrp->candInv(i) ? ~oriValue : oriValue;

         if (hash.check(CirSimValue(value), queryGrp)) {
            queryGrp->candidates().emplace_back(g, oriValue != queryGrp->repValue());
         } else {
            queryGrp = new CirFecGrp;
            queryGrp->candidates().emplace_back(g);
            hash.forceInsert(CirSimValue(oriValue, oriGrp->candInv(i)), queryGrp);
            lCandGrp.push_back(queryGrp);
         }
      }
      delete *iter;

      // Collect valid FEC groups (i.e. size > 1)
      for (auto iter2 = lCandGrp.begin(); iter2 != lCandGrp.end(); ++iter2)
         if ((*iter2)->isValid())
            _lFecGrps.push_front(*iter2);
         else
            delete *iter2;
   }
}

void 
CirMgr::sim_sortFecGrps_var()
{
   for (CirFecGrp* grp : _lFecGrps)
      grp->sort();

   _lFecGrps.sort(
      [] (const CirFecGrp* g1, const CirFecGrp* g2) {
         return g1->repVar() < g2->repVar();
      });
}

void 
CirMgr::sim_linkGrp2Gate()
{  
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
      if (_vAllGates[i])
         _vAllGates[i]->setGrp(nullptr);
   }
   for (CirFecGrp* grp : _lFecGrps) {
      for (unsigned i = 0, n = grp->size(); i < n; ++i) {
         grp->candGate(i)->setGrp(grp);
         grp->candGate(i)->setGrpIdx(i);
      }
   }
}

void 
CirMgr::sim_writeSimLog(const unsigned nPatterns) const
{
   if (!_simLog) return;
   unsigned i, j;
   for (i = 0; i < nPatterns; ++i) {
      for (j = 0; j < _nPI; ++j)
         *_simLog << pi(j)->value(i);
      *_simLog << ' ';
      for (j = 0; j < _nPO; ++j)
         *_simLog << po(j)->value(i);
      *_simLog << endl;
   }
}

void 
CirMgr::sim_printMsg_totalFecGrp() const
{
   cout << flush << "\rTotal #FEC Group = " << _lFecGrps.size();
}