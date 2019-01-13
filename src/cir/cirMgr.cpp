/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
unsigned globalRef = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   // Open aag file
   ifstream fin(fileName.c_str(), ios::in);
   if (!fin) {
      cout << "Cannot open design \"" << fileName << "\"!!\n";
      return false;
   }

   // Parse
   if (!parse_aag(fin))    return false;
   if (!parse_pi(fin))     return false;
   if (!parse_po(fin))     return false;
   if (!parse_aig(fin))    return false;
   if (!parse_symbol(fin)) return false;

   parse_comment(fin); // if any

   // Build Lists
   buildDfsList();
   buildFloatingList();
   buildUnusedList();
   buildUndefList();
   countAig();

   sortAllGateFanout();

   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   static const int FRONTWIDTH = 7;
   static const int BACKWIDTH  = 9;
   cout << endl;
   cout << "Circuit Statistics\n";
   cout << "==================\n";
   cout << setw(FRONTWIDTH) << left  << "  PI"
        << setw(BACKWIDTH)  << right << _nPI  << endl;
   cout << setw(FRONTWIDTH) << left  << "  PO"
        << setw(BACKWIDTH)  << right << _nPO  << endl;
   cout << setw(FRONTWIDTH) << left  << "  AIG"
        << setw(BACKWIDTH)  << right << _nAIG << endl;
   cout << "------------------\n";
   cout << setw(FRONTWIDTH) << left  << "  Total"
        << setw(BACKWIDTH)  << right << (_nPI + _nPO + _nAIG) 
        << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _vDfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (unsigned i = 0; i < _nPI; ++i)
      cout << " " << pi(i)->var();
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (unsigned i = 0; i < _nPO; ++i)
      cout << " " << po(i)->var();
   cout << endl;
}

void
CirMgr::printFloatGates() const
{ 
   // Print floating gates, if any
   if (!_vFloatingList.empty()) {
      cout << "Gates with floating fanin(s):";  
      for (unsigned i = 0, n = _vFloatingList.size(); i < n; ++i)
         cout << " " << _vFloatingList[i]->var();
      cout << endl;
   }
   // Print unuse gates, if any
   if (!_vUnusedList.empty()) {
      cout << "Gates defined but not used  :";  
      for (unsigned i = 0, n = _vUnusedList.size(); i < n; ++i)
         cout << " " << _vUnusedList[i]->var();
      cout << endl;
   }
}

void
CirMgr::printFECPairs() const
{
   unsigned i, n;
   unsigned cnt = 0;
   for (const CirFecGrp* grp : _lFecGrps) {
      cout << "[" << cnt++ << "]";
      for (i = 0, n = grp->size(); i < n; ++i)
         cout << (grp->candInv(i) ? " !" : " ") 
              << grp->candGate(i)->var();
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   unsigned i, n;
   // First line
   outfile << "aag"   << " " << _maxIdx << " " << _nPI     << " " 
           << _nLATCH << " " << _nPO    << " " << _nDfsAIG << endl;
   // PIs
   for (i = 0; i < _nPI; ++i) 
      outfile << LIT(pi(i)->var(), 0) << endl;
   // POs
   for (i = 0; i < _nPO; ++i) 
      outfile << LIT(po(i)->fanin0_var(), po(i)->fanin0_inv()) 
              << endl;
   // AIGs
   for (i = 0, n = _vDfsList.size(); i < n; ++i)
      if (_vDfsList[i]->isAig())
         outfile << LIT(_vDfsList[i]->var(), 0)     << " "
                 << LIT(_vDfsList[i]->fanin0_var(),
                        _vDfsList[i]->fanin0_inv()) << " "
                 << LIT(_vDfsList[i]->fanin1_var(),
                        _vDfsList[i]->fanin1_inv()) << endl;
   // Symbols
   for (i = 0; i < _nPI; ++i)
      if (pi(i)->symbol() != "")
         outfile << "i" << i << " " << pi(i)->symbol() << endl;
   for (i = 0; i < _nPO; ++i)
      if (po(i)->symbol() != "")
         outfile << "o" << i << " " << po(i)->symbol() << endl;
   // Comments (optional)
   outfile << "c" << endl;
   // outfile << "AAG output by Chen-Hao Hsu" << endl;
   outfile << "AAG output by Chung-Yang (Ric) Huang" << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   ++globalRef;
   vector<unsigned> vPiGates(0), vAigGates(0);
   rec_writeGate(g, vPiGates, vAigGates);
   unsigned maxId = 0;
   if (!vPiGates.empty())
      maxId = std::max(maxId, *std::max_element(vPiGates.begin(), vPiGates.end()));
   if (!vAigGates.empty())
      maxId = std::max(maxId, *std::max_element(vAigGates.begin(), vAigGates.end()));

   // First line
   outfile << "aag " << maxId << " " << vPiGates.size() << " 0 1 " << vAigGates.size() << endl;
   // PIs
   std::sort(vPiGates.begin(), vPiGates.end(), std::less<unsigned>());
   for (unsigned i = 0, n = vPiGates.size(); i < n; ++i)
      outfile << LIT(vPiGates[i], 0) << endl;
   // POs
   outfile << LIT(g->var(), 0) << endl;
   // AIGs
   CirGate* curGate = nullptr;
   for (unsigned i = 0, n = vAigGates.size(); i < n; ++i) {
      curGate = _vAllGates[vAigGates[i]];
      outfile << LIT(curGate->var(), 0) << " "
              << LIT(curGate->fanin0_var(), curGate->fanin0_inv()) << " "
              << LIT(curGate->fanin1_var(), curGate->fanin1_inv()) << endl;
   }
   // symbols
   for (unsigned i = 0, n = vPiGates.size(); i < n; ++i)
      if (static_cast<const CirPiGate*>(_vAllGates[vPiGates[i]])->symbol() != "")
         outfile << "i" << i << " " << static_cast<const CirPiGate*>(_vAllGates[vPiGates[i]])->symbol() << endl;
   outfile << "o0 Gate_" << g->var() << endl;
   // Comments (optional)
   outfile << "c" << endl;
   // outfile << "Write gate (" << g->var() << ") by Chen-Hao Hsu" << endl;
   outfile << "Write gate (" << g->var() << ") by Chung-Yang (Ric) Huang" << endl;
}

void
CirMgr::rec_writeGate(CirGate *g, vector<unsigned>& vPiGates, vector<unsigned>& vAigGates) const
{
   if (g == nullptr) return;
   if (g->ref() == globalRef) return;
   if (g->isUndef()) return;
   g->setRef(globalRef);
   rec_writeGate(g->fanin0_gate(), vPiGates, vAigGates);
   rec_writeGate(g->fanin1_gate(), vPiGates, vAigGates);
   if (g->isPi()) 
      vPiGates.emplace_back(g->var());
   else if (g->isAig())
      vAigGates.emplace_back(g->var());
}

/**********************************************************/
/*   class CirMgr member functions for building lists     */
/**********************************************************/
void
CirMgr::rec_dfs(CirGate* g)
{
   if (g == nullptr) return;
   if (g->ref() == globalRef) return;
   if (g->isUndef()) return;
   g->setRef(globalRef);
   rec_dfs(g->fanin0_gate());
   rec_dfs(g->fanin1_gate());
   _vDfsList.push_back(g);
   if (g->isAig()) ++_nDfsAIG;
}

void
CirMgr::buildDfsList()
{
   _nDfsAIG = 0;
   ++globalRef;
   _vDfsList.clear();
   for (unsigned i = 0; i < _nPO; ++i)
      rec_dfs(po(i));
}

void
CirMgr::buildFloatingList()
{
   _vFloatingList.clear();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->isFloating())
            _vFloatingList.push_back(_vAllGates[i]);
}

void
CirMgr::buildUnusedList()
{
   _vUnusedList.clear();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->nFanouts() == 0) 
            if (_vAllGates[i]->isAig() || _vAllGates[i]->isPi())
               _vUnusedList.push_back(_vAllGates[i]);
}

void
CirMgr::buildUndefList()
{
   _vUndefList.clear();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->isUndef())
            _vUndefList.push_back(_vAllGates[i]);
}

void
CirMgr::countAig()
{
   _nAIG = 0;
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->isAig())
            ++_nAIG;
}

void 
CirMgr::sortAllGateFanout()
{
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         _vAllGates[i]->sortFanout();
}

/**********************************************************/
/*   class CirMgr member functions for freeing pointers   */
/**********************************************************/
void
CirMgr::delGate(CirGate* g)
{
   _vAllGates[g->var()] = nullptr;
   delete g;
}

void
CirMgr::clear()
{
   // Delete gates
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         delete _vAllGates[i];
   _vAllGates.clear();

   // Delete FEC groups
   for (CirFecGrp* grp : _lFecGrps)
      delete grp;
   _lFecGrps.clear();
}

/**********************************************************/
/*   class CirMgr member functions for merging two gates  */
/**********************************************************/
void 
CirMgr::mergeGate(CirGate* aliveGate, CirGate* deadGate, bool invMerged) 
{
   // Remove deadGate from deadGate's fanins' fanout
   deadGate->fanin0_gate()->rmFanout(deadGate);
   deadGate->fanin1_gate()->rmFanout(deadGate);

   // Replace deadGate's fanouts' fanin with aliveGate,
   // and also add to aliveGate's fanout
   for (unsigned i = 0, n = deadGate->nFanouts(); i < n; ++i) {
      deadGate->fanout_gate(i)->replaceFanin(aliveGate, deadGate->fanout_inv(i) ^ invMerged, deadGate);
      aliveGate->addFanout(deadGate->fanout_gate(i), deadGate->fanout_inv(i) ^ invMerged);
   }
   // Delete deadGate
   delGate(deadGate);
}

/**********************************************************/
/*   class CirMgr member functions for utility            */
/**********************************************************/
string 
CirMgr::bitString(size_t s) const
{
   size_t n = 8 * sizeof(size_t);
   string str(n, 'x');
   for (size_t i = 0, k = 1; i < n; ++i) {
      str[n - i - 1] = s & k ? '1' : '0';
      k <<= 1;
   }
   return str;
}