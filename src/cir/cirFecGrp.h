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

   size_t   value()   const { return _value;                 }
   unsigned size()    const { return _candidates.size();     }
   bool     isValid() const { return _candidates.size() > 1; }

   void setValue(size_t v)      { _value = v;               }
   void push(const CirGateV& c) { _candidates.push_back(c); }

   vector<CirGateV>& candidates() { return _candidates; }

   // Candidates access functions
   const CirGateV& cand(const unsigned i)     const { return _candidates[i];                                  }
   CirGate*        candGate(const unsigned i) const { return _candidates[i].gate();                           }
   bool            candInv(const unsigned i)  const { return _candidates[i].isInv() ^ _candidates[0].isInv(); }
   unsigned        candVar(const unsigned i)  const { return _candidates[i].gate()->var();                    }

   // Representation of this group access functions
   const CirGateV& rep()     const { return _candidates[0];                                  }
   CirGate*        repGate() const { return _candidates[0].gate();                           }
   bool            repInv()  const { return _candidates[0].isInv() ^ _candidates[0].isInv(); }
   unsigned        repVar()  const { return _candidates[0].gate()->var();                    }

   // Sort by var in incresing order
   void sort() {
      std::sort(_candidates.begin(), _candidates.end(), 
         [] (const CirGateV& g1, const CirGateV& g2) {
               return g1.gate()->var() < g2.gate()->var();
         });
   }

   void reset() { 
      _candidates.resize(0); 
      _value = 0; 
      _candidates.shrink_to_fit(); 
   }

private:
   vector<CirGateV> _candidates;
   size_t           _value;
};

#endif // CIRFECGRP_H