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
   CirFecGrp() {}
   ~CirFecGrp() {}

   unsigned size()    const { return _candidates.size();             }
   bool     isValid() const { return _candidates.size() > 1;         }

   void push(const CirGateV& c) { _candidates.push_back(c); }
   void emplace_back(CirGate* g = nullptr, size_t inv = 0) { _candidates.emplace_back(g, inv); }

   vector<CirGateV>& candidates() { return _candidates; }
   const vector<CirGateV>& candidates() const { return _candidates; }

   // Candidates access functions
   CirGateV&       cand(const unsigned i)           { return _candidates[i];                                  }
   const CirGateV& cand(const unsigned i)     const { return _candidates[i];                                  }
   CirGate*        candGate(const unsigned i) const { return _candidates[i].gate();                           }
   bool            candInv(const unsigned i)  const { return _candidates[i].isInv() ^ _candidates[0].isInv(); }
   unsigned        candVar(const unsigned i)  const { return _candidates[i].gate()->var();                    }

   // Representation of this group access functions
   CirGateV&       rep()            { return _candidates[0];                                  }
   const CirGateV& rep()      const { return _candidates[0];                                  }
   CirGate*        repGate()  const { return _candidates[0].gate();                           }
   bool            repInv()   const { return _candidates[0].isInv() ^ _candidates[0].isInv(); }
   unsigned        repVar()   const { return _candidates[0].gate()->var();                    }
   size_t          repValue() const { return _candidates[0].gate()->value();                  }

   // Lazy delete
   void lazy_delete(const unsigned i) {
      assert(i < _candidates.size());
      _candidates[i] = CirGateV(nullptr);
   }

   // Refine _candidates
   void refine() {
    _candidates.erase(remove_if(
      _candidates.begin(), _candidates.end(),
      [&](const CirGateV gv){
         return gv.gate() == nullptr;
      }), _candidates.end());
   }

   // Sort by var in incresing order
   void sort() {
      std::sort(_candidates.begin(), _candidates.end(), 
         [] (const CirGateV& g1, const CirGateV& g2) {
               return g1.gate()->var() < g2.gate()->var();
         });
   }

   // Sort by dfsOrder in incresing order
   void sortDfsOrder() {
      std::sort(_candidates.begin(), _candidates.end(), 
         [] (const CirGateV& g1, const CirGateV& g2) {
               return ((CirAigGate*)g1.gate())->dfsOrder() < ((CirAigGate*)g2.gate())->dfsOrder();
         });
   }

   void reset() { 
      _candidates.resize(0); 
   }

private:
   vector<CirGateV> _candidates;
};

#endif // CIRFECGRP_H