/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

/**********************************************************/
/*   Type Declaration                                     */
/**********************************************************/
class CirGate;
class CirMgr;
class SatSolver;

/**********************************************************/
/*   Macros                                               */
/**********************************************************/
#define VAR(x)     (x >> 1)
#define INV(x)     (x & 0x01)
#define LIT(x, y)  ((x << 1) + y)

#define BYTE_SIZE  (8)
#define SIZET_SIZE (sizeof(size_t) * BYTE_SIZE)
#define NEG        (0x1)

/**********************************************************/
/*   Constants                                            */
/**********************************************************/
constexpr size_t CONST1           = (0x1);
constexpr size_t CONST0           = (0x0);
constexpr size_t INV_CONST1       = (~(0x1));
constexpr size_t ALL1             = (~(0x0));
constexpr size_t SIM_CYCLE        = (sizeof(size_t) * BYTE_SIZE);
constexpr size_t HALF_SIM_CYCLE   = ((sizeof(size_t) * BYTE_SIZE) / 2);

/**********************************************************/
/*   Global enum                                          */
/**********************************************************/

#endif // CIR_DEF_H
