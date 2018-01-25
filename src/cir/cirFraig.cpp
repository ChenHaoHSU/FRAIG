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
	/*  HashSet Implementation  */
	/****************************/
/*
	HashSet<CirStrashS> hashS;
	hashS.init(getHashSize(_vDfsList.size()));

	CirStrashS queryS;
	for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
		// Skip non-AIG gate
		if (!_vDfsList[i]->isAig()) continue;

		queryS.setGate(_vDfsList[i]);
		if (hashS.query(queryS)) {
			fprintf(stdout, "Strashing: %d merging %d...\n", queryS.gate()->var(), _vDfsList[i]->var());
			cout << "Strashing: " << queryS.gate()->var() 
			     << " merging "   << _vDfsList[i]->var() 
			     << "...\n";
			mergeGates(queryS.gate(), _vDfsList[i], false);
			delGate(_vDfsList[i]);
		}
		else 
			hashS.insert(queryS);
	}
*/

	/****************************/
	/*  HashMap Implementation  */
	/****************************/
	HashMap<CirStrashM, CirGate*> hashM;
	hashM.init(getHashSize(_vDfsList.size()));

	CirStrashM keyM;     // hash key
	CirGate* valueM = 0; // value
	for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
		// Skip non-AIG gate
		if (!_vDfsList[i]->isAig()) continue;

		keyM.setGate(_vDfsList[i]);
		if (hashM.check(keyM, valueM)) {
			fprintf(stdout, "Strashing: %d merging %d...\n", valueM->var(), _vDfsList[i]->var());
			// cout << "Strashing: " << valueM->var() 
			//      << " merging "   << _vDfsList[i]->var() 
			//      << "...\n";
			mergeGates(valueM, _vDfsList[i], false);
			delGate(_vDfsList[i]);
		}
		else 
			hashM.insert(keyM, _vDfsList[i]);
	}

	// Update Lists
   buildDfsList();
   buildFloatingList();
   countAig();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
