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
      nPatterns += SIM_CYCLE;
      ++nFail;
   }

   cout << nPatterns << " patterns simulated.\n";
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
   model.reset();

   while (patternFile >> patternStr) {

      // Check if pattern is valid
      if (!checkPattern(patternStr)) {
         cout << (nPatterns - periodCnt) << " patterns simulated.\n";
         return;
      }

      // Set pattern value to model
      for (i = 0; i < _nPI; ++i) {
         if (patternStr[i] == '0')
            model.add0(i, periodCnt);
         else // patternStr[i] == '1'
            model.add1(i, periodCnt);
      }
      ++nPatterns;
      ++periodCnt;

      // Simulate immediately, if 64 patterns are collected.
      if (periodCnt >= SIM_CYCLE) {
         for (i = 0; i < model.size(); ++i)
            pi(i)->setValue(model[i]);
         simulation();
         periodCnt = 0;
         model.reset();
      }
   }

   // If nPatterns % SIM_CYCLE(64) != 0, simulate here one more time
   if (periodCnt % SIM_CYCLE != 0) {
      for (i = 0; i < model.size(); ++i)
         pi(i)->setValue(model[i]);
      simulation();
   }

   cout << nPatterns << " patterns simulated.\n";
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
   for (unsigned i = 0, n = _vDfsList.size(); i < n; ++i)
      _vDfsList[i]->calValue();



}

