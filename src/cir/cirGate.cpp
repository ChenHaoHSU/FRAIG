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
   // Information string
   string infoStr = getTypeStr() + "(" + to_string(_var) + ")";
   if (isPi()) {
      if (((CirPiGate*)this)->symbol() != "")
         infoStr += "\"" + ((CirPiGate*)this)->symbol() + "\"";
   }
   if (isPo()) {
      if (((CirPoGate*)this)->symbol() != "")
         infoStr += "\"" + ((CirPoGate*)this)->symbol() + "\"";
   }
   infoStr += ", line " + to_string(_lineNo);

   // FEC string
   string fecStr = "FECs:"   + this->fecStr();

   // Value string
   string valStr = "Value: " + this->valueStr();

   cout << "================================================================================\n";
   cout << "= " << infoStr << endl;
   cout << "= " << fecStr  << endl;
   cout << "= " << valStr  << endl;
   cout << "================================================================================\n";
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
   if (g == nullptr) return;
   if (level < 0) return;
   for (int i = 0; i < nSpace; ++i) cout << ' ';
   cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->var();
   if (g->ref() == globalRef && level > 0) {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if (level > 0 && g->isAig()) g->setRef(globalRef);
   rec_rptFanin(g->fanin0_gate(), g->fanin0_inv(), level - 1, nSpace + 2);
   rec_rptFanin(g->fanin1_gate(), g->fanin1_inv(), level - 1, nSpace + 2);
}

void
CirGate::rec_rptFanout(const CirGate* g, bool inv, int level, int nSpace) const 
{
   if (g == nullptr) return;
   if (level < 0) return;
   for (int i = 0; i < nSpace; ++i) cout << ' ';
   cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->var();
   if (g->ref() == globalRef && level > 0) {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if (level > 0 && g->isAig()) g->setRef(globalRef);
   for (unsigned i = 0, n = g->nFanouts(); i < n; ++i) 
      rec_rptFanout(g->fanout_gate(i), g->fanout_inv(i), level - 1, nSpace + 2);
}

string 
CirGate::fecStr() const
{
   string str = "";
   if (_grp != nullptr) {
      bool inv;
      const vector<CirGateV>& vCands = _grp->candidates();
      const CirGateV thisGateV = _grp->cand(_grpIdx);
      for(auto& gateV : vCands) {
         if (gateV != thisGateV) {
            inv = thisGateV.isInv() ^ gateV.isInv();
            str = str + " " + (inv ? "!" : "") + to_string(gateV.gate()->var());
         }
      }
   }
   return str;
}

string 
CirGate::valueStr() const
{
   static const unsigned nBit     = 64;
   static const unsigned nCluster = 8;
   string str = "";
   size_t value_copy = _value;
   for (unsigned i = 0; i < nBit; ++i) {
      if (!(i % nCluster) && i != 0) str = "_" + str;
      if (value_copy & CONST1)
         str = "1" + str;
      else
         str = "0" + str;
      value_copy >>= 1;
   }
   return str;
}

/**************************************/
/*   class CirGate sorting functions  */
/**************************************/
void 
CirGate::sortFanout() 
{ 
   sort(_fanouts.begin(), _fanouts.end(), 
        [] (const CirGateV& g1, const CirGateV& g2) {
           return g1.gate()->var() < g2.gate()->var();
        });
}

/*******************************************/
/*   class CirGate Fanin Fanout operation  */
/*******************************************/
bool 
CirGate::replaceFanin(CirGate* newFanin, bool newInv, CirGate* oldFanin)
{
   if (_fanin0.gate() == oldFanin) {
      _fanin0.setGateV(newFanin, newInv);
      return true;
   }
   if (_fanin1.gate() == oldFanin) {
      _fanin1.setGateV(newFanin, newInv);
      return true;
   }
   return false;
}

bool 
CirGate::replaceFanout(CirGate* newFanin, bool newInv, CirGate* oldFanin)
{
   for (unsigned i = 0; i < _fanouts.size(); ++i) {
      if (_fanouts[i].gate() == oldFanin) {
         _fanouts[i].setGateV(newFanin, newInv);
         return true;
      }
   }
   return false;
}

bool 
CirGate::rmFanout(CirGate* g)
{
   for (unsigned i = 0; i < _fanouts.size(); ++i) {
      if (_fanouts[i].gate() == g) {
         _fanouts[i] = _fanouts.back();
         _fanouts.pop_back();
         return true;
      }
   }
   return false;
}
