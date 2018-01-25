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
   // Trivial case (no PI, no Simulation)
   if (_nPI == 0) return;

   unsigned i, j;

   // Collect all SimValue strings
   vector<string> patternStrings;
   string patternStr;
   int nPatterns = 0;
   while (getline(patternFile, patternStr, '\n')) {
      ++nPatterns;
      // Length check
      if (patternStr.length() != _nPI) {
         cerr << "\nError: Pattern(" << patternStr << ") length(" << patternStr.size() 
              << ") does not match the number of inputs(" << _nPI << ") in a circuit!!\n";
         cout << "0 patterns simulated.\n";
         return;
      }
      // Bit check
      for (i = 0; i < patternStr.length(); ++i) {
         if (patternStr[i] != '0' && patternStr[i] != '1') {
            cerr << "Error: Pattern(" << patternStr << ") contains a non-0/1 character(\'" 
                 << patternStr[i] << "\').";
            cout << "0 patterns simulated.\n";
            return;
         }
      }
      // Collect pattern
      patternStrings.push_back(patternStr);
   }

   // Set PI values
   for (i = 0; i < patternStrings.size(); ++i) {
      for (j = 0; j < patternStrings[i].length(); ++j) {
         if (patternStrings[i][j] == '0')
            pi(j)->addPattern0();
         else 
            pi(j)->addPattern1();
      }
   }

   cout << nPatterns << " patterns simulated.\n";
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
