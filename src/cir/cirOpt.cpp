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
	for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i)
		_vDfsList[i]->setRef(globalRef);

	CirGate* g = 0;
	for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
		if (_vAllGates[i]) {
			g = _vAllGates[i];
			if (g->ref() != globalRef) {
				if (g->getTypeStr() == "AIG") {
            	g->fanin0_gate()->rmFanout(g);
            	g->fanin1_gate()->rmFanout(g);
	            cout << "Sweeping: AIG(" << g->var() << ") removed...\n";
	            _vGarbageList.push_back(_vAllGates[i]);
	            _vAllGates[i] = 0;
				}
				else if (g->getTypeStr() == "UNDEF") {
	            cout << "Sweeping: UNDEF(" << g->var() << ") removed...\n";
	            _vGarbageList.push_back(_vAllGates[i]);
	            _vAllGates[i] = 0;
	         }
	         else {}
	      }
	   }
	}

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
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
