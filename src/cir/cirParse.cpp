/****************************************************************************
  FileName     [ cirParse.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Parsing functions for AAG file ]
  Author       [ Chen-Hao Hsu ]
  Date         [ 2018/1/17 created ]
****************************************************************************/

#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern unsigned lineNo = 0;  // in printint, lineNo needs to ++
extern unsigned colNo = 0;   // in printing, colNo needs to ++
extern char buf[1024];
extern string errMsg;
extern int errInt;
extern CirGate *errGate;

bool 
CirMgr::parseAag(ifstream& fin)
{
   lineNo = 0;
   string aagStr;
   fin >> aagStr;
   fin >> _maxIdx;
   fin >> _nPI;
   fin >> _nLATCH;
   fin >> _nPO;
   fin >> _nAIG;
   ++lineNo;
   preProcess();
   return true;
}

void  
CirMgr::preProcess()
{
   // Resize _vAllGates, and reserve _vGarbageList
   _vAllGates.resize(1 + _maxIdx + _nPO, 0);
   _vGarbageGates.reserve(_vAllGates.size());
   // Create CONST gate
   CirConstGate* newGate = new CirConstGate;
   _vAllGates[0] = newGate;
}

bool 
CirMgr::parsePi(ifstream& fin)
{
   CirPiGate* newPiGate = 0;
   unsigned   lit       = 0;
   for (unsigned i = 0; i < _nPI; ++i) {
      fin >> lit;
      newPiGate = new CirPiGate(++lineNo, VAR(lit));
      _vAllGates[VAR(lit)] = newPiGate;
      _vPi.push_back(newPiGate);
   }
   return true;
}

bool 
CirMgr::parsePo(ifstream& fin)
{
   CirPoGate* newPoGate = 0;
   CirGate*   fanin     = 0;
   unsigned   lit       = 0;
   for (unsigned i = 0; i < _nPO; ++i) {
      fin >> lit;
      newPoGate = new CirPoGate(++lineNo, (_maxIdx + 1 + i));
      fanin = queryGate(VAR(lit));
      newPoGate->setFanin0(fanin, INV(lit));
      fanin->addFanout(newPoGate, INV(lit));
      _vAllGates[_maxIdx + 1 + i] = newPoGate;
   }
   return true;
}

bool 
CirMgr::parseAig(ifstream& fin)
{
   unsigned g_lit, f0_lit, f1_lit;
   CirGate *g, *f0, *f1;
   for (unsigned i = 0; i < _nAIG; ++i) {
      fin >> g_lit;
      fin >> f0_lit;
      fin >> f1_lit;
      g  = queryGate(VAR(g_lit));
      f0 = queryGate(VAR(f0_lit));
      f1 = queryGate(VAR(f1_lit));
      g->setFanin0(f0, INV(f0_lit));
      g->setFanin1(f1, INV(f1_lit));
      f0->addFanout(g, INV(f0_lit));
      f1->addFanout(g, INV(f1_lit));
      g->setLineNo(++lineNo);
   }
   return true;
}

bool 
CirMgr::parseSymbol(ifstream& fin)
{
   string str = "";
   int    idx = 0;
   while (fin >> str) {
      if (str == "c") break;
      if (str[0] == 'i') {
         str = str.substr(1);
         myStr2Int(str, idx);
         fin >> str;
         pi(idx)->setSymbol(str);
      }
      if (str[0] == 'o') {
         str = str.substr(1);
         myStr2Int(str, idx);
         fin >> str;
         po(idx)->setSymbol(str);
      }
   }
   return true;
}

bool 
CirMgr::parseComment(ifstream& fin)
{
   return true;
}

CirGate* 
CirMgr::queryGate(unsigned gid)
{
   assert(gid < _vAllGates.size());
   if (_vAllGates[gid] != 0) return _vAllGates[gid];

   // Create new aig gate
   CirAigGate* newGate = new CirAigGate(0, gid);
   _vAllGates[gid] = newGate;
   return _vAllGates[gid];
}









