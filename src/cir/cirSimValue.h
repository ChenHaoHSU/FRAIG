/****************************************************************************
  FileName     [ cirSimValue.h ]
  PackageName  [ cir ]
  Synopsis     [ CirSimValue class for key of HashMap ]
  Author       [ Chen-Hao Hsu ]
  Date         [ 2018/2/2 created ]
****************************************************************************/

#ifndef CIRSIMVALUE_H
#define CIRSIMVALUE_H

using namespace std;

class CirInitSimValue;
class CirSimValue;

class CirInitSimValue
{
public:
   CirInitSimValue(size_t v = 0, bool inv = false) {
      _value = inv ? (~v) : (v);
   }
   ~CirInitSimValue() {}

   size_t value() const { return _value; }
   void setValue(size_t v, bool inv = false) {  _value = inv ? (~v) : (v); }

   size_t operator() () const { return _value * (~_value); }
   bool operator == (const CirInitSimValue& k) const { return _value == k.value() || (~_value) == k.value(); }

private:
   size_t _value;
};

class CirSimValue
{
public:
   CirSimValue(size_t v = 0, bool inv = false) {
      _value = inv ? (~v) : (v);
   }
   ~CirSimValue() {}

   size_t value() const { return _value; }
   void setValue(size_t v, bool inv = false) {  _value = inv ? (~v) : (v); }

   size_t operator() () const { return _value; }
   bool operator == (const CirSimValue& k) const { return _value == k.value(); }

private:
   size_t _value;
};
#endif // CIRSIMVALUE_H