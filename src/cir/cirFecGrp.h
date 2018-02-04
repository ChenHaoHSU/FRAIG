/****************************************************************************
  FileName     [ cirFecGrp.h ]
  PackageName  [ cir ]
  Synopsis     [ Functionally equivalent group (FecGrp) class ]
  Author       [ Chen-Hao Hsu ]
  Date         [ 2018/2/2 created ]
****************************************************************************/

#ifndef CIRFECGRP_H
#define CIRFECGRP_H

#include <vector>
#include <algorithm>
#include "cirGate.h"
#include "cirDef.h"

using namespace std;

class CirFecGrp
{
public:
   CirFecGrp(size_t v = 0) : _value(v) {}
	~CirFecGrp() {}

   vector<CirGateV>& candidates() { return _candidates; }

   const CirGateV operator[] (size_t i) const { return _candidates[i]; }
   CirGateV& operator[] (size_t i) { return _candidates[i]; }

   void setValue(size_t v) { _value = v; }
   void reset() { _candidates.resize(0); _value = 0; _candidates.shrink_to_fit(); }
   void push(const CirGateV& c) { _candidates.push_back(c); }
   size_t value() const { return _value; }
   unsigned size() const { return _candidates.size(); }
   bool isValid() const { return _candidates.size() > 1; }
   void sort() {
      std::sort(_candidates.begin(), _candidates.end(), 
         [] (const CirGateV& g1, const CirGateV& g2) {
               return g1.gate()->var() < g2.gate()->var();
         });
   }

private:
   vector<CirGateV> _candidates;
   size_t           _value;
};

#endif // CIRFECGRP_H