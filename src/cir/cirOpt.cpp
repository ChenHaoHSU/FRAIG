/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern unsigned globalRef;


/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   // Mark all gates in DFS List
   ++globalRef;
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      _vDfsList[i]->setRef(globalRef);
      // [Note] UNDEF gates never exist in DFS list.
      //        So, make sure that UNDEF gates which 
      //        can be reached by POs will be marked.
      if (_vDfsList[i]->isPo()) {
         _vDfsList[i]->fanin0_gate()->setRef(globalRef);
      } else if (_vDfsList[i]->isFloating()) { 
         _vDfsList[i]->fanin0_gate()->setRef(globalRef);
         _vDfsList[i]->fanin1_gate()->setRef(globalRef);
      } else {}
   }

   // Sweeping unmarked gates
   CirGate* g = nullptr; // for convenient
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
      if (_vAllGates[i] && (g = _vAllGates[i])) {
         if (g->ref() != globalRef) {
            // sweep AIG
            if (g->isAig()) {
               g->fanin0_gate()->rmFanout(g);
               g->fanin1_gate()->rmFanout(g);
               fprintf(stdout, "Sweeping: AIG(%d) removed...\n", g->var());
               delGate(g);
            }
            // sweep UNDEF
            else if (g->isUndef()) {
               fprintf(stdout, "Sweeping: UNDEF(%d) removed...\n", g->var());
               delGate(g);
            }
            else {}
         }
      }
   }

   // Update Lists
   buildFloatingList();
   buildUnusedList();
   buildUndefList();
   countAig();

   sortAllGateFanout();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   CirGate* g = nullptr;
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      // Skip non-AIG gate
      if (!_vDfsList[i]->isAig()) continue;

      g = _vDfsList[i];
      // Apply trivial optimization (4 cases)
      //    Case1: One of fanins is const0
      //    Case2: One of fanins is const1
      //    Case3: Two fanins are the same (var) and in the same phase
      //    Case4: Two fanins are the same (var) but in inverting phase
      // 
      if ( g->fanin0_gate() == constGate() ) {
         if ( !g->fanin0_inv() ) { 
            // case1
            mergeGate(constGate(), g, false);
            fprintf(stdout, "Simplifying: %u merging %u...\n", constGate()->var(), g->var());
         } else{
            // case2
            mergeGate(g->fanin1_gate(), g, g->fanin1_inv());
            fprintf(stdout, "Simplifying: %u merging %s%u...\n", 
               g->fanin1_gate()->var(), (g->fanin1_inv() ? "!" : ""), g->var());
         }
      } else if ( g->fanin1_gate() == constGate() ) {
         if ( !g->fanin1_inv() ) { 
            // case1
            mergeGate(constGate(), g, false);
            fprintf(stdout, "Simplifying: %u merging %u...\n", constGate()->var(), g->var());
         } else{ 
            // case2
            mergeGate(g->fanin0_gate(), g, g->fanin0_inv());
            fprintf(stdout, "Simplifying: %u merging %s%u...\n", 
               g->fanin0_gate()->var(), (g->fanin0_inv() ? "!" : ""), g->var());
         }
      } else if ( g->fanin0_gate() == g->fanin1_gate() ) {
         if ( g->fanin0_inv()  == g->fanin1_inv() ) { 
            // case3
            mergeGate(g->fanin0_gate(), g, g->fanin0_inv());
            fprintf(stdout, "Simplifying: %u merging %s%u...\n", 
               g->fanin0_gate()->var(), (g->fanin0_inv() ? "!" : ""), g->var());
         } else { 
            // case4
            mergeGate(constGate(), g, false);
            fprintf(stdout, "Simplifying: %u merging %u...\n", constGate()->var(), g->var());
         }
      } else {} // no optimization
   }

   // Update Lists
   buildDfsList();
   buildFloatingList();
   buildUnusedList();
   buildUndefList();
   countAig();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/