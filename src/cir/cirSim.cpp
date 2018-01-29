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




}

void
CirMgr::fileSim(ifstream& patternFile)
{
   // Load patternFile
   vector<string> vPatternStrings;
   if (!loadPatternFile(patternFile, vPatternStrings)) {
      cout << "0 patterns simulated.\n";
      return;
   }

   // Set PI values
   unsigned i;
   unsigned totalCnt  = 0;
   unsigned periodCnt = 0; // SIM_CYCLE
   unsigned nPatterns = vPatternStrings.size();
   while (totalCnt < nPatterns) {
      vector<size_t> model(_nPI, CONST0);
      for (periodCnt = 0; periodCnt < SIM_CYCLE && totalCnt < nPatterns; ++periodCnt, ++totalCnt) {
         for (i = 0; i < _nPI; ++i) {
            if (vPatternStrings[totalCnt][i] == '0')
               model[i] &= ~(CONST1 << periodCnt);
            else // '1'
               model[i] |=  (CONST1 << periodCnt);
         }
      }
      // Set model to PIs
      for (i = 0; i < model.size(); ++i) {
         pi(i)->setValue(model[i]);
      }
   }

   // Simulate
   simulation();

   cout << nPatterns << " patterns simulated.\n";
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
bool
CirMgr::loadPatternFile(ifstream& patternFile, vector<string>& vPatternStrings)
{
   // Error Handling:
   //    1. Length of pattern string == nPI
   //    2. Pattern string consists of '0' or '1'.
   // 
   vPatternStrings.clear();
   string patternStr;
   unsigned i;
   while (patternFile >> patternStr) {
      // 1. Length check
      if (patternStr.length() != _nPI) {
         cerr << "\nError: Pattern(" << patternStr << ") length(" << patternStr.size() 
              << ") does not match the number of inputs(" << _nPI << ") in a circuit!!\n";
         return false;
      }
      // 2. Char check
      for (i = 0; i < patternStr.length(); ++i) {
         if (patternStr[i] != '0' && patternStr[i] != '1') {
            cerr << "Error: Pattern(" << patternStr << ") contains a non-0/1 character(\'" 
                 << patternStr[i] << "\').";
            return false;
         }
      } 
      // Collect pattern
      vPatternStrings.emplace_back(patternStr);
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

