/****************************************************************************
  FileName     [ cirModel.h ]
  PackageName  [ cir ]
  Synopsis     [ Model class for simulation patterns ]
  Author       [ Chen-Hao Hsu ]
  Date         [ 2018/1/30 created ]
****************************************************************************/

#ifndef CIRMODEL_H
#define CIRMODEL_H

#include <vector>
#include <cstdlib>
#include <cassert>
#include "cirDef.h"

using namespace std;

class CirModel
{
public:
   CirModel() {}
   CirModel(unsigned n) { 
      _patterns.resize(n, 0); 
      srand((unsigned)time(NULL));
   }
   ~CirModel() {}

   const size_t operator[] (size_t i) const { return _patterns[i]; }
   size_t& operator[] (size_t i) { return _patterns[i]; }

   void reset() { fill(_patterns.begin(), _patterns.end(), 0); }
   void resize(const unsigned n) { _patterns.clear(); _patterns.resize(n, 0); }
   size_t size() const { return _patterns.size(); }

   void add0(const unsigned i, const unsigned pos) { _patterns[i] &= ~(CONST1 << pos); }
   void add1(const unsigned i, const unsigned pos) { _patterns[i] |=  (CONST1 << pos); }

   void random() {
      for (size_t& s : _patterns)
         s = (s << (HALF_SIM_CYCLE)) ^ rand();
   }

   void setPattern(const string& patternStr, const unsigned pos) {
      for (unsigned i = 0, n = _patterns.size(); i < n; ++i) {
         if (patternStr[i] == '0')
            _patterns[i] &= ~(CONST1 << pos);
         else if (patternStr[i] == '1')
            _patterns[i] |=  (CONST1 << pos);
         else assert(false);
      }
   }

private:
   vector<size_t> _patterns;
}; 

#endif // CIRMODEL_H