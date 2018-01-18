/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGateV;
class CirGate;
class CirPiGate;
class CirPoGate;
class CirAigGate;
class CirConstGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateV
{
public:
  #define NEG 0x1
  CirGateV(CirGate* g = 0, size_t phase = 0):  _gateV(size_t(g) + phase) {}
  CirGate* gate() const { return (CirGate*)(_gateV & ~size_t(NEG)); }
  bool isInv() const { return (_gateV & NEG); }
  bool operator == (const CirGateV& i) const { return (gate() == i.gate() && isInv() == i.isInv()); }
  bool null() const { return _gateV == 0; }

private:
  size_t _gateV;
};


class CirGate
{
public:
   CirGate(string t = "", string s = "", unsigned l = 0, unsigned v = 0)
      : _typeStr(t), _symbol(s), _lineNo(l), _var(v), _ref(0), _value(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   string   getTypeStr()   const { return _typeStr;       }
   string   typeStr()      const { return _typeStr;       }
   string   symbol()       const { return _symbol;        }
   unsigned getLineNo()    const { return _lineNo;        }
   unsigned lineNo()       const { return _lineNo;        }
   unsigned var()          const { return _var;           }
   unsigned ref()          const { return _ref;           }
   size_t   value()        const { return _value;         }

   CirGateV fanin0()       const { return _fanin0;        }
   CirGateV fanin1()       const { return _fanin1;        }
   CirGate* fanin0_gate()  const { return _fanin0.gate(); }
   CirGate* fanin1_gate()  const { return _fanin1.gate(); }
   bool     fanin0_inv()   const { return _fanin0.isInv();}
   bool     fanin1_inv()   const { return _fanin1.isInv();}

   CirGateV fanout(unsigned i)      const { assert(i < _fanouts.size()); return _fanouts[i];         }
   CirGate* fanout_gate(unsigned i) const { assert(i < _fanouts.size()); return _fanouts[i].gate();  }
   bool     fanout_inv(unsigned i)  const { assert(i < _fanouts.size()); return _fanouts[i].isInv(); }

   unsigned nFanouts()     const { return _fanouts.size();  }
   unsigned bFanoutEmpty() const { return _fanouts.empty(); }

   virtual bool isAig()      const = 0;
   virtual bool isFloating() const = 0;
   virtual bool isUndef()    const = 0;

   // Basic setting methods
   void setTypeStr(string s)            { _typeStr = s;                       }
   void setSymbol(string s)             { _symbol = s;                        }
   void setLineNo(unsigned l)           { _lineNo = l;                        }
   void setVar(unsigned v)              { _var = v;                           }
   void setRef(unsigned r)              { _ref = r;                           }
   void setFanin0(const CirGateV& g)    { _fanin0 = g;                        } 
   void setFanin1(const CirGateV& g)    { _fanin1 = g;                        } 
   void addFanout(const CirGateV& g)    { _fanouts.push_back(g);              } 
   void setFanin0(CirGate* g, size_t i) { _fanin0 = CirGateV(g, i);           } 
   void setFanin1(CirGate* g, size_t i) { _fanin1 = CirGateV(g, i);           } 
   void addFanout(CirGate* g, size_t i) { _fanouts.push_back(CirGateV(g, i)); } 

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate()                            const;
   void reportFanin(int level)                  const;
   void reportFanout(int level)                 const;
   void rec_rptFanin(CirGate*, bool, int, int)  const;
   void rec_rptFanout(CirGate*, bool, int, int) const;
   
private:
   string           _typeStr;
   string           _symbol;
   unsigned         _lineNo;
   unsigned         _var;
   unsigned         _ref;

protected:
   CirGateV         _fanin0;
   CirGateV         _fanin1;
   vector<CirGateV> _fanouts;
   size_t           _value; // simulation value
};


class CirPiGate : public CirGate
{
public:
   CirPiGate(unsigned l = 0, unsigned v = 0)
      : CirGate("PI", "", l, v) {}
   ~CirPiGate() {}

   virtual bool isAig()      const { return false; }
   virtual bool isFloating() const { return false; }
   virtual bool isUndef()    const { return false; }

   virtual void printGate() const {
      cout << "PI  " << var();
      if (symbol() != "") cout << " (" << symbol() << ")";
      cout << endl;
   }

private:
};


class CirPoGate : public CirGate
{
public:
   CirPoGate(unsigned l = 0, unsigned v = 0)
      : CirGate("PO", "", l, v) {}
   ~CirPoGate() {}

   virtual bool isAig()      const { return false;          }
   virtual bool isFloating() const { return _fanin0.null(); }
   virtual bool isUndef()    const { return false;          }

   virtual void printGate() const {
      cout << "PO  " << var() << " "
           << (_fanin0.gate()->isUndef() ? "*" : "") << (_fanin0.isInv() ? "!" : "") 
           << _fanin0.gate()->var();
      if (symbol() != "") cout << " (" << symbol() << ")";
      cout << endl;
   }
   
private:
};


class CirAigGate : public CirGate
{
public:
   CirAigGate(unsigned l = 0, unsigned v = 0)
      : CirGate("AIG", "", l, v) {}
   ~CirAigGate() {}

   virtual bool isAig()      const { return true; }
   virtual bool isFloating() const { 
      if (isUndef()) return false;
      return _fanin0.gate()->isUndef() || _fanin1.gate()->isUndef();
   }
   virtual bool isUndef()   const { return _fanin0.null() || _fanin1.null(); }

   virtual void printGate() const {
      assert(!isUndef());
      cout << "AIG " << var() << " "
           << (_fanin0.gate()->isUndef() ? "*" : "") << (_fanin0.isInv() ? "!" : "") 
           << _fanin0.gate()->var() << " " 
           << (_fanin1.gate()->isUndef() ? "*" : "") << (_fanin1.isInv() ? "!" : "") 
           << _fanin1.gate()->var() << endl;
   }
   
private:
};


class CirConstGate : public CirGate
{
public:
   CirConstGate()
      : CirGate("CONST", "", 0, 0) {}
   ~CirConstGate() {}

   virtual bool isAig()      const { return false; }
   virtual bool isFloating() const { return false; }
   virtual bool isUndef()    const { return false; }

   virtual void printGate() const {
      cout << "CONST0" << endl;
   }
   
private:
};

#endif // CIR_GATE_H
