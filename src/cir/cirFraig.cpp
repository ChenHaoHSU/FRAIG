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

extern unsigned globalRef;

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
   SatSolver satSolver;
   while (!_lFecGrps.empty()) {
      fraig_initSatSolver(satSolver);
      fraig_assignDfsOrder();
      fraig_sortFecGrps_dfsOrder();
      




      buildDfsList();
   }

   strash();
   assert(_lFecGrps.empty());
   _bFirstSim = false;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::fraig_initSatSolver(SatSolver& satSolver)
{
   satSolver.initialize();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      satSolver.newVar();
}

void
CirMgr::fraig_assignDfsOrder()
{
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isAig()) continue;
      ((CirAigGate*)_vDfsList[i])->setDfsOrder(i+1);
   }
   constGate()->setDfsOrder(0);
}

void
CirMgr::fraig_sortFecGrps_dfsOrder()
{
   for(auto& grp : _lFecGrps)
      grp->sortDfsOrder();
}

bool 
CirMgr::fraig_solve(const CirGateV& g1, const CirGateV& g2, SatSolver& satSolver)
{
   Var newV = satSolver.newVar();
   satSolver.addXorCNF(newV, g1.gate()->var(), g1.isInv(), 
                          g2.gate()->var(), g2.isInv());
   fraig_proveMsg(g1, g2);
   satSolver.assumeRelease();
   satSolver.assumeProperty(newV, true);  
   satSolver.assumeProperty(constGate()->var(), false);
   return satSolver.assumpSolve();
}

void
CirMgr::fraig_proveMsg(const CirGateV& g1, const CirGateV& g2)
{
   bool inv = g1.isInv() ^ g2.isInv();
   if(g1.gate() == constGate())
      cout << flush << '\r' << "Prove " << (inv ? "!" : "") << g2.gate()->var() << " = 1..." ;
   else
      cout << flush << '\r' << "Prove (" << g1.gate()->var() << ", " 
           << (inv ? "!" : "") << g2.gate()->var() << ")...";
}

void
CirMgr::fraig_collectConuterEx(const SatSolver& satSolver, CirModel& model, const unsigned pos)
{
   for (unsigned i = 0; i < _nPI; ++i) {
      if (satSolver.getValue(pi(i)->var()) == 0)
         model.add0(i, pos);
      else if (satSolver.getValue(pi(i)->var()) == 1)
         model.add1(i, pos);
      else assert(false); // should not return -1...
   }
}

void
CirMgr::fraig_mergeEqGates(vector<pair<CirGateV, CirGateV> >& vMergePairs)
{
   // pair<CirGateV, CirGateV> : pair<liveGate, deadGate>
   cout << flush << '\r';
   for (unsigned i = 0, n = vMergePairs.size(); i < n; ++i) {
      cout << "Fraig: "<< vMergePairs[i].first.gate()->var() <<" merging " 
           << (vMergePairs[i].first.isInv() ^ vMergePairs[i].second.isInv() ? "!" : "") 
           << vMergePairs[i].second.gate()->var() << "..." << endl;
      mergeGate(vMergePairs[i].first.gate(), vMergePairs[i].second.gate(),
         vMergePairs[i].first.isInv() ^ vMergePairs[i].second.isInv());
   }
}