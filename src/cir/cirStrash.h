/****************************************************************************
  FileName     [ cirStrash.h ]
  PackageName  [ cir ]
  Synopsis     [ Classes for Strash command. 
                 CirStrashS is the key-value pair for HashSet implementation.
                 CirStrashM is the key for HashSet implementation. ]
  Author       [ Chen-Hao Hsu ]
  Date         [ 2018/1/25 created ]
****************************************************************************/

#ifndef CIRSTRASH_H
#define CIRSTRASH_H

#include <iostream>

using namespace std;

/******************************
   HashSet implementation
******************************/
class CirStrashS
{
public:
	CirStrashS(CirGate* g = 0) : _gate(g) {}
	~CirStrashS() {}

	/* Key */
	size_t operator () () const {
		return _gate->fanin0() ^ _gate->fanin1();
		// return _gate->fanin0() + _gate->fanin1();
	}

	/* Key comparison function */
	bool operator == (const CirStrashS& c) const {
		if (_gate->fanin0() == c.gate()->fanin0())
			return (_gate->fanin1() == c.gate()->fanin1());
		if (_gate->fanin0() == c.gate()->fanin1())
			return (_gate->fanin1() == c.gate()->fanin0());
		return false;
	}

	CirStrashS& operator = (const CirStrashS& c) {
		_gate = c.gate();
		return (*this);
	}

	/* Basic functions */
	CirGate* gate() const { return _gate; }
	void     setGate(CirGate* g) { _gate = g; }

private:
	CirGate* _gate;
};

/******************************
   HashMap implementation
******************************/
class CirStrashM
{
public:
	CirStrashM(CirGate* g = 0) : _gate(g) {}
	~CirStrashM() {}

	/* Key */
   size_t operator() () const { 
		return _gate->fanin0() ^ _gate->fanin1();
		// return _gate->fanin0() + _gate->fanin1();
	}

	/* Key comparison function */
   bool operator == (const CirStrashM& c) const { 
		if (_gate->fanin0() == c.gate()->fanin0())
			return (_gate->fanin1() == c.gate()->fanin1());
		if (_gate->fanin0() == c.gate()->fanin1())
			return (_gate->fanin1() == c.gate()->fanin0());
		return false;
   }

	/* Basic functions */
	CirGate* gate() const { return _gate; }
	void     setGate(CirGate* g) { _gate = g; }

private:
	CirGate* _gate;
};

#endif // CIRSTRASH_H


