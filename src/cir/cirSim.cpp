/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cmath>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHashMap.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   unsigned max_fail  = 200;

   unsigned i         = 0;
   unsigned nPatterns = 0;
   unsigned nFail     = 0; // fail to make #fecGrps change

   CirModel model(_nPI);

   while (nFail < max_fail) {
      model.random();
      for (i = 0; i < model.size(); ++i)
         pi(i)->setValue(model[i]);
      simulation();
      cout << flush << '\r' << "Total #FEC Group = " << _lFecGrps.size();
      nPatterns += SIM_CYCLE;
      ++nFail;
   }

   sortFecGrps();
   linkGrp2Gate();

   cout << flush << '\r' << nPatterns << " patterns simulated.\n";
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   // Set PI values
   unsigned i;
   unsigned periodCnt = 0; // SIM_CYCLE, a period
   unsigned nPatterns = 0;
   string patternStr;

   CirModel model(_nPI);
   
   while (true) {

      if ( !(patternFile >> patternStr) ) {
         // simulate here one more time, then break
         if (periodCnt != 0) {
            for (i = 0; i < model.size(); ++i)
               pi(i)->setValue(model[i]);
            simulation();
            cout << flush << '\r' << "Total #FEC Group = " << _lFecGrps.size();
            nPatterns += periodCnt;
            cout << nPatterns << " " << periodCnt << endl;
         }
         break;
      }

      // Check if pattern is valid
      if (!checkPattern(patternStr)) break;

      // Set pattern value to model
      for (i = 0; i < _nPI; ++i) {
         if (patternStr[i] == '0')
            model.add0(i, periodCnt);
         else // patternStr[i] == '1'
            model.add1(i, periodCnt);
      }
      ++periodCnt;

      // Simulate immediately, if 64 patterns are collected.
      if (periodCnt == SIM_CYCLE) {
         for (i = 0; i < model.size(); ++i)
            pi(i)->setValue(model[i]);
         simulation();
         cout << flush << '\r' << "Total #FEC Group = " << _lFecGrps.size();
         nPatterns += periodCnt;
         periodCnt = 0;
         model.reset();
      }
   }

   sortFecGrps();
   linkGrp2Gate();

   cout << flush << '\r' << nPatterns << " patterns simulated.\n";
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
bool
CirMgr::checkPattern(const string& patternStr)
{
   // Error Handling:
   //    1. Length of pattern string == nPI
   //    2. Pattern string consists of '0' or '1'.
   // 
   // 1. Length check
   if (patternStr.length() != _nPI) {
      cerr << "\nError: Pattern(" << patternStr << ") length(" << patternStr.size() 
           << ") does not match the number of inputs(" << _nPI << ") in a circuit!!\n";
      return false;
   }
   // 2. Char check
   for (unsigned i = 0; i < patternStr.length(); ++i) {
      if (patternStr[i] != '0' && patternStr[i] != '1') {
         cerr << "Error: Pattern(" << patternStr << ") contains a non-0/1 character(\'" 
              << patternStr[i] << "\').\n";
         return false;
      }
   } 
   return true;
}

void 
CirMgr::simulation() 
{
   // Calculate sim value
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         _vAllGates[i]->calValue();

   // Classify gates into FEC groups
   if (!_bFirstSim) {
      initClassifyFecGrp();
      _bFirstSim = true;
   }
   else 
      classifyFecGrp();

   refineFecGrp();
}

void 
CirMgr::initClassifyFecGrp()
{
   HashMap<CirInitSimValue, CirFecGrp*> hash;
   CirGate* g = 0;
   CirFecGrp* queryGrp = 0;
   hash.init(getHashSize(_vAllGates.size()));
   // Const gate !!!
   queryGrp = getNewFecGrp();
   queryGrp->setValue(constGate()->value());
   queryGrp->candidates().emplace_back(constGate());
   hash.insert(CirInitSimValue(constGate()->value()), queryGrp);
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
      if (!_vAllGates[i]) continue;
      if (!_vAllGates[i]->isAig()) continue;
      g = _vAllGates[i];
      if (hash.check(CirInitSimValue(g->value()), queryGrp)) {
         queryGrp->candidates().emplace_back(g, g->value() != queryGrp->value());
      }
      else {
         queryGrp = getNewFecGrp();
         queryGrp->setValue(g->value());
         queryGrp->candidates().emplace_back(g);
         hash.insert(CirInitSimValue(g->value()), queryGrp);
      }
   }
}

void 
CirMgr::classifyFecGrp()
{
   unsigned i, n;
   bool       firstInv = false;
   size_t     oriValue = 0;
   size_t     value    = 0;
   CirGate*   g        = 0;
   CirFecGrp* queryGrp = 0;
   for (auto iter = _lFecGrps.begin(); iter != _lFecGrps.end();) {
      CirFecGrp& oriGrp = (*(*iter));
      HashMap<CirSimValue, CirFecGrp*> hash;
      hash.init(getHashSize(oriGrp.size()));
      firstInv = oriGrp[0].isInv();
      for (i = 0, n = oriGrp.size(); i < n; ++i) {
         g = oriGrp[i].gate();
         oriValue = g->value();
         value = oriGrp[i].isInv() ^ firstInv ? ~oriValue : oriValue;
         if (hash.check(CirSimValue(value), queryGrp)) {
            queryGrp->candidates().emplace_back(g, oriValue != queryGrp->value());
         }
         else {
            queryGrp = getNewFecGrp();
            queryGrp->setValue(g->value());
            queryGrp->candidates().emplace_back(g);
            hash.insert(CirSimValue(oriValue, oriGrp[i].isInv()), queryGrp);
         }
      }
      // Recycle ...
      delFecGrp((*iter));
      iter = _lFecGrps.erase(iter);
   }
}

void 
CirMgr::refineFecGrp()
{
   // Remove invalid FEC groups (i.e. size < 2)
   // 
   for (auto iter = _lFecGrps.begin(); iter != _lFecGrps.end();) {
      CirFecGrp& grp = *(*iter);
      if (grp.isValid()) ++iter;
      else {
         assert(grp.size() == 1);
         grp[0].gate()->setGrp(0);
         delFecGrp((*iter)); // recycle
         iter = _lFecGrps.erase(iter);
      }
   }
}

void 
CirMgr::sortFecGrps()
{
   for (auto& grp : _lFecGrps)
      grp->sort();

   _lFecGrps.sort(
      [] (const CirFecGrp* g1, const CirFecGrp* g2) {
         return (*g1)[0].gate()->var() < (*g2)[0].gate()->var();
      });
}

void 
CirMgr::linkGrp2Gate()
{  
   int i, n;
   for (i = 0, n = _vAllGates.size(); i < n; ++i) 
      if (_vAllGates[i])
         _vAllGates[i]->setGrp(0);
   for (auto& grp : _lFecGrps)
      for (i = 0, n = grp->size(); i < n; ++i)
         (*grp)[i].gate()->setGrp(grp);
}

void 
CirMgr::delFecGrp(CirFecGrp* g)
{
   _vGarbageFecGrps.push_back(g);
}

CirFecGrp* 
CirMgr::getNewFecGrp()
{
   CirFecGrp* ret = 0;
   if (!_vGarbageFecGrps.empty()) {
      ret = _vGarbageFecGrps.back();
      ret->reset();
      _vGarbageFecGrps.pop_back();
   } 
   else {
      ret = new CirFecGrp;
   }
   _lFecGrps.push_front(ret);
   return ret;
}
