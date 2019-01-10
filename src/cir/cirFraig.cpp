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
   CirGate* valueM = nullptr; // value
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
   #define UNSAT_MERGE_LIMIT 10000

   SatSolver satSolver;
   CirModel model(_nPI);
   unsigned periodCnt = 0; // SIM_CYCLE, a period
   unsigned unsatCnt = 0;
   vector<pair<CirGateV, CirGateV> > vMergePairs;

   while (!_lFecGrps.empty()) {
      fraig_initSatSolver(satSolver);
      fraig_refine_fecgrp();
      fraig_assignDfsOrder();
      fraig_sortFecGrps_dfsOrder();
      
      for (unsigned dfsId = 0, n = _vDfsList.size(); dfsId < n; ++dfsId) {
         CirGate* curGate = _vDfsList[dfsId];

         // Skip functionally unique gates
         if (!curGate->isAig()) continue;

         satSolver.addAigCNF(curGate->var() + 1, curGate->fanin0_var() + 1, curGate->fanin0_inv(), 
                                                 curGate->fanin1_var() + 1, curGate->fanin1_inv());
         if (curGate->grp() == nullptr) continue;

         // Check if rep gate is curGate itself => no need to check
         CirFecGrp* fecGrp = curGate->grp();
         CirGate* repGate = fecGrp->repGate();
         if (repGate == curGate) continue;
         assert(fecGrp->candGate(curGate->grpIdx()) == curGate);

         // Use SATsolver to prove if repGate and curGate are equivalent
         const CirGateV& repGateV = fecGrp->rep();
         const CirGateV& curGateV = fecGrp->cand(curGate->grpIdx());
         assert(repGateV.gate() != curGateV.gate());

         const bool result = fraig_solve(repGateV, curGateV, satSolver);
         /* 
          * SAT:
          * repGate and curGateare are functionally INequivalent 
         */
         if (result) {
            // Get the SAT asssignments from SATsolver and store the pattern in model
            for (unsigned i = 0, nPI = _vPi.size(); i < nPI; ++i) {
               const int val = satSolver.getValue(_vPi[i]->var() + 1);
               if (val == 0) {
                  model.add0(i, periodCnt);
               }
               else
                  model.add1(i, periodCnt);
            }
            
            // Counter goes on
            ++periodCnt;

            // Simulate the circuit if SIM_CYCLE(64) patterns are already collected
            if (periodCnt >= SIM_CYCLE) {
               sim_simulation(model);
               sim_linkGrp2Gate();
               periodCnt = 0;
               cout << flush << '\r' << "Updating by SAT... Total #FEC Group = " << _lFecGrps.size() << endl;
               break;
            }
         }
         /* 
          * UNSAT:
          * repGate and curGate are functionally equivalent 
         */
         else {
            // Record the merge pair, lazy merge
            vMergePairs.emplace_back(repGateV, curGateV); // repGateV alive; curGateV dead
            fecGrp->cand(curGate->grpIdx()) = CirGateV(nullptr);
            if (++unsatCnt >= UNSAT_MERGE_LIMIT) {
               fraig_mergeEqGates(vMergePairs);
               unsatCnt = 0;
               cout << flush << '\r' << "Updating by UNSAT... Total #FEC Group = " << _lFecGrps.size() << endl;
               break;
            }
         }
      }

      buildDfsList();
   }


   if (unsatCnt > 0) {
      fraig_mergeEqGates(vMergePairs);
      cout << flush << '\r' << "Updating by UNSAT... Total #FEC Group = " << _lFecGrps.size() << endl;
   }
   if (periodCnt > 0) {
      sim_simulation(model);
      cout << flush << '\r' << "Updating by SAT... Total #FEC Group = " << _lFecGrps.size() << endl;
   }
   
   buildDfsList();
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
   /* Initialize SAT solver, #var in SAT solver will be the same as _vAllGates 
      [Note] gate.var <==> SATsolver[gate.var+1]
            e.g. for gate 5, its variable id is (5+1) in SAT solver
   */
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
   constGate()->setDfsOrder(0); // important!! No one can merge const gate!!
}

void
CirMgr::fraig_sortFecGrps_dfsOrder()
{
   for(auto& grp : _lFecGrps)
      grp->sortDfsOrder();
   sim_linkGrp2Gate();
}

bool 
CirMgr::fraig_solve(const CirGateV& g1, const CirGateV& g2, SatSolver& satSolver)
{
   Var newV = satSolver.newVar();
   satSolver.addXorCNF(newV, g1.gate()->var()+1, g1.isInv(), 
                             g2.gate()->var()+1, g2.isInv());
   fraig_proveMsg(g1, g2);
   satSolver.assumeRelease();
   satSolver.assumeProperty(newV, true);
   satSolver.assumeProperty(1, false);
   bool result = satSolver.assumpSolve();
   cout << (result ? "SAT" : "UNSAT");
   return result;
}

void
CirMgr::fraig_proveMsg(const CirGateV& g1, const CirGateV& g2)
{
   const bool inv = g1.isInv() ^ g2.isInv();
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
      assert(vMergePairs[i].first.gate()->dfsOrder() < vMergePairs[i].second.gate()->dfsOrder());
      mergeGate(vMergePairs[i].first.gate(), vMergePairs[i].second.gate(),
         vMergePairs[i].first.isInv() ^ vMergePairs[i].second.isInv());
   }
   fraig_refine_fecgrp();
   vMergePairs.clear();
}

void
CirMgr::fraig_refine_fecgrp() {
   for (auto& grp : _lFecGrps)
      grp->refine();
   sim_sweepInvalidFecGrp();
}
