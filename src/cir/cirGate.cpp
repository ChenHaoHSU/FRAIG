/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <cstdlib>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
extern unsigned globalRef;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	static const int WIDTH = 46;

	// Information string
	string infoStr = getTypeStr() + "(" + to_string(_var) + ")";
	if (getTypeStr() == "PI") {
		if (((CirPiGate*)this)->symbol() != "")
			infoStr += "\"" + ((CirPiGate*)this)->symbol() + "\"";
	}
	if (getTypeStr() == "PO") {
		if (((CirPoGate*)this)->symbol() != "")
			infoStr += "\"" + ((CirPoGate*)this)->symbol() + "\"";
	}
	infoStr += ", line " + to_string(_lineNo);

	// FEC string
	string fecStr = "FECs:";

	// Value string
	string valStr = "Value: " + valueStr();

	cout << "==================================================\n";
	cout << "= " << setw(WIDTH) << left << infoStr       << " =\n";
	cout << "= " << setw(WIDTH) << left << fecStr        << " =\n";
	cout << "= " << setw(WIDTH) << left << valStr        << " =\n";
	cout << "==================================================\n";
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   ++globalRef;
   rec_rptFanin(this, 0, level, 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   ++globalRef;
   rec_rptFanout(this, 0, level, 0);
}

void
CirGate::rec_rptFanin(const CirGate* g, bool inv, int level, int nSpace) const
{
	if (g == 0) return;
	if (level < 0) return;
	for (int i = 0; i < nSpace; ++i) cout << ' ';
	cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->var();
	if (g->ref() == globalRef && level > 0) {
		cout << " (*)" << endl;
		return;
	}
	cout << endl;
	if (level > 0 && g->getTypeStr() == "AIG") g->setRef(globalRef);
	rec_rptFanin(g->fanin0_gate(), g->fanin0_inv(), level - 1, nSpace + 2);
	rec_rptFanin(g->fanin1_gate(), g->fanin1_inv(), level - 1, nSpace + 2);
}

void
CirGate::rec_rptFanout(const CirGate* g, bool inv, int level, int nSpace) const 
{
	if (g == 0) return;
	if (level < 0) return;
	for (int i = 0; i < nSpace; ++i) cout << ' ';
	cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->var();
	if (g->ref() == globalRef && level > 0) {
		cout << " (*)" << endl;
		return;
	}
	cout << endl;
	if (level > 0 && g->getTypeStr() == "AIG") g->setRef(globalRef);
	for (unsigned i = 0, n = g->nFanouts(); i < n; ++i) 
		rec_rptFanout(g->fanout_gate(i), g->fanout_inv(i), level - 1, nSpace + 2);
}

string 
CirGate::valueStr() const
{
	static const unsigned nBit = 32;
	string str = "";
	size_t value_copy = _value;
	for (unsigned i = 0; i < nBit; ++i) {
		if (!(i % 4) && i != 0) str = "_" + str;
		if (value_copy & CONST1)
			str = "1" + str;
		else
			str = "0" + str;
	}
	return str;
}

/**************************************/
/*   class CirGate sorting functions  */
/**************************************/
bool 
CirGate::fanoutSort(const CirGateV& g1, const CirGateV& g2)
{
   return g1.gate()->var() < g2.gate()->var();
}

void 
CirGate::sortFanout() 
{ 
   sort(_fanouts.begin(), _fanouts.end(), fanoutSort); 
}

void 
CirGate::rmFanout(CirGate* g)
{
	for (unsigned i = 0; i < _fanouts.size(); ++i) {
		if (_fanouts[i].gate() == g) {
			_fanouts[i] = _fanouts.back();
			_fanouts.pop_back();
		}
	}
}
