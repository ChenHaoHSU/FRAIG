/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashSet.h"
#include "myHashMap.h"
#include "util.h"
#include "cirStrash.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   /****************************/
   /*  HashMap Implementation  */
   /****************************/
   HashMap<CirStrashM, CirGate*> hashM;
   hashM.init(getHashSize(_vDfsList.size()));

   CirStrashM keyM;     // hash key
   CirGate* valueM = 0; // value
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isAig()) continue; // Skip non-AIG gate
      keyM.setGate(_vDfsList[i]);
      if (hashM.check(keyM, valueM)) {
         fprintf(stdout, "Strashing: %d merging %d...\n", 
            valueM->var(), _vDfsList[i]->var());
         mergeGate(valueM, _vDfsList[i], false);
      }
      else 
         hashM.forceInsert(keyM, _vDfsList[i]);
   }

   /****************************/
   /*  HashSet Implementation  */
   /****************************/
/*
   HashSet<CirStrashS> hashS;
   hashS.init(getHashSize(_vDfsList.size()));

   CirStrashS queryS;
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isAig()) continue; // Skip non-AIG gate
      queryS.setGate(_vDfsList[i]);
      if (hashS.query(queryS)) {
         fprintf(stdout, "Strashing: %d merging %d...\n", 
            queryS.gate()->var(), _vDfsList[i]->var());
         mergeGate(queryS.gate(), _vDfsList[i], false);
      }
      else 
         hashS.insert(queryS);
   }
*/
   // Update Lists
   buildDfsList();
   buildFloatingList();
   countAig();
}

void
CirMgr::fraig()
{
   unsigned i, n;
   CirFecGrp* curGrp = 0;
   CirGate* repGate = 0;
   while (!_lFecGrps.empty()) {
      buildDfsList();
      genProofModel();
      assignDfsOrder();
      sortFecGrps_dfsOrder();
      for (auto iter = _lFecGrps.begin(); iter != _lFecGrps.end(); iter = _lFecGrps.erase(iter)) {
         curGrp = *iter;
         vector<CirGateV>& cands = curGrp->candidates();
         for (i = 1, n = curGrp->size(); i < n; ++i) {
            // if (solve(cands[0], cands[i])) {
               
            // }
            // else {

            // }
         }
      }
      // mergeGates();
   }
   assert(_lFecGrps.empty());
   _bFirstSim = false;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::genProofModel()
{
   _satSolver.initialize();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      _satSolver.newVar();

   CirGate* g = 0;
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isAig()) continue;
      g = _vDfsList[i];
      _satSolver.addAigCNF(g->var(), 
                           g->fanin0_var(), g->fanin0_inv(),
                           g->fanin1_var(), g->fanin1_inv());
   }
}

void
CirMgr::assignDfsOrder()
{
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isAig()) continue;
      ((CirAigGate*)_vDfsList[i])->setDfsOrder(i);
   }
}

void
CirMgr::sortFecGrps_dfsOrder()
{
   for(auto& grp : _lFecGrps)
      grp->sortDfsOrder();
}
